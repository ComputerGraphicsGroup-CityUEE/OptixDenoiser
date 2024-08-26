#include <stdio.h>
#include <stdarg.h>


#include <stdlib.h>
#include <memory.h>
#include <string.h>


#include "g_common.h"


#include "g_pmd.h"




GPmd::GPmd()
{
  memset( this, 0, sizeof(GPmd) );
}

GPmd::~GPmd()
{
  int i;
  for( i=0; i<n_line; i++ )
    SAFE_FREE( line[i] );
  SAFE_FREE( line );
  n_line=0;
}

void GPmd::save( const char *spath )
{
  FILE *f0 = fopen( spath, "wt" );
    for( int i=0; i<n_line; i++ )
      fprintf( f0, "%s", line[i] );
  fclose(f0);
}


char* GPmd::setTag( const char *var_name, const char *format, ... )
{
  char *var_line;
  char tmp_format[256];

  va_list va;
  va_start(va, format);

    sprintf( tmp_format, "%s %s\n", var_name, format );

    var_line = vsquery( var_name, "", 0 );
    if( var_line )
    {
      vsprintf( var_line, tmp_format, va );
    }else
    {
      char str[256];
      vsprintf( str, tmp_format, va );
      addline( str );
      var_line = line[n_line-1];
    }

  va_end(va);

  return var_line;
}

int GPmd::query_count( const char *var_name )
{
  int i, count = 0;

  size_t var_name_len = strlen(var_name);
  for( i=0; i<n_line; i++ )
  {
    if(
      line[i][0]!='#' &&
      strlen(line[i]) > var_name_len && 
      memcmp( line[i], var_name, var_name_len ) == 0 && 
      (
        line[i][var_name_len]==' ' ||
        line[i][var_name_len]=='\t'
      )
    ){
        count++;
    }
  }

  return count;
}
int vsscanf ( const char * s, const char * format, va_list arg );

char* GPmd::query_index( const char *var_name, int idx, const char *format,  ... )
{
  int i, count;
  char tmp_format[256];
  size_t var_name_len = strlen(var_name);

  count = 0;
  for( i=0; i<n_line; i++ )
  {
    if(
      line[i][0]!='#' &&
      strlen(line[i]) > var_name_len && 
      memcmp( line[i], var_name, var_name_len ) == 0 && 
      (
        line[i][var_name_len]==' ' ||
        line[i][var_name_len]=='\t'
      )
    ){
      if( count!=idx )
      {
        count++;
      }else
      {
        sprintf( tmp_format, "%s %s", var_name, format );
        va_list args;
        va_start(args, format);
        vsscanf( line[i], tmp_format, args );
        va_end(args);
        return line[i++];
      }
    }
  }

  printf( "[Error] GPmd::query_index, index out of bound.\n" );
  exit(-1);
}


char* GPmd::query_array( const char *var_name, const char *format,  ... )
{
  int slot;

  for( slot=0; slot<16; slot++ )
    if( strcmp( qa_tag[slot], var_name )==0 )
      break;

  if( slot==16 )
    for( slot=0; slot<16; slot++ )
      if( strcmp( qa_tag[slot], "" )==0 )
      {
        query( var_name, "" );
        strcpy( qa_tag[slot], var_name );
        qa_idx[slot] = 0;
        break;
      }

  if( slot==16 )
  {
    printf( "[Error] GPmd::query_array, all 16 slot are used\n" );
    exit(-1);
  }

  char tmp_format[256];
  size_t var_name_len = strlen(var_name);

  for( int &i=qa_idx[slot]; i<n_line; i++ )
  {
    if(
      line[i][0]!='#' &&
      strlen(line[i]) > var_name_len && 
      memcmp( line[i], var_name, var_name_len ) == 0 && 
      (
        line[i][var_name_len]==' ' ||
        line[i][var_name_len]=='\t'
      )
    ){
      sprintf( tmp_format, "%s %s", var_name, format );
      va_list args;
      va_start(args, format);
      vsscanf( line[i], tmp_format, args );
      va_end(args);
      return line[i++];
    }
  }

  qa_tag[slot][0] = '\0';
  qa_idx[slot] = 0;
  return NULL;
}


char* GPmd::query( const char *var_name, const char *format,  ... )
{
  va_list args;
  va_start(args, format);
  char *res = vsquery( var_name, format, args );
  va_end(args);

  if( res )
    return res;

  printf( "[Error] GPmd::query, tag %s not found.\n", var_name );
  exit(-1);
}

char* GPmd::soft_query( const char *var_name, const char *format,  ... )
{
  va_list args;
  va_start(args, format);
  char *res = vsquery( var_name, format, args );
  va_end(args);
  return res;
}

char* GPmd::soft_query_bool( const char *var_name, bool *value )
{
  char str[256]="";
  char *res = soft_query( var_name, "%s", str );
  if( res )
  {
    _strlwr(str);
    if( strcmp(str,"true")==0 )
      *value = true;
    else
      *value = false;
  }
  return res;
}

char* GPmd::vsquery( const char *var_name, const char *format, char *va )
{
  if( query_count(var_name) )
  {
    char tmp_format[256];
    size_t var_name_len = strlen(var_name);

    for( int i=0; i<n_line; i++ )
    {
      if(
        line[i][0]!='#' &&
        strlen(line[i]) > var_name_len && 
        memcmp( line[i], var_name, var_name_len ) == 0 && 
        (
          line[i][var_name_len]==' ' ||
          line[i][var_name_len]=='\t'
        )
      ){
        sprintf( tmp_format, "%s %s", var_name, format );
        vsscanf( line[i], tmp_format, va );
        return line[i];
      }
    }
  }
  return NULL;
}


char* GPmd::query_path( const char *var_name, char *spath )
{
  char str[256];
    query( var_name, "%s", str );

  if( ( str[0]=='/' && str[1]=='/') || str[1]==':' )
  {
    strcpy( spath, str );
    replace_char( spath, '\\', '/' );
  }else
  {
    GPath gp = parse_spath( pmd_path );
    sprintf( spath, "%s%s", gp.dname, str );
  }
  return spath;
}

char* GPmd::query_dir( const char *var_name, char *spath )
{
  query_path( var_name, spath );

  if( spath[ strlen(spath)-1 ] != '/' )
  {
    spath[ strlen(spath) ] = '/';
    spath[ strlen(spath)+1 ] = '\0';
  }

  return spath;
}


void GPmd::addline( const char *str )
{
  line = (char**)realloc( line, (n_line+1)*sizeof(char*) );
  line[n_line] = (char*) malloc( (strlen(str)+1)*sizeof(char) );
  strcpy( line[n_line], str );

  n_line++;
}

void GPmd::load( const char *spath )
{
  int i;
  char str[256]="";

  strcpy( pmd_path, spath );

  for (i = 0; i < n_line; i++)
	SAFE_FREE(line[i]);
    SAFE_FREE( line );
  n_line=0;
  
  {
    FILE *f0 = fopen( spath, "rt" );
    if( f0==NULL )
    {
      printf( "[Error] GPmd::load(), file %s not found.\n", spath );
      exit(-1);
    }

    while( fgets( str, 256, f0 ) )
      addline( str );

    fclose(f0);
  }
}


//void query_defined( const char *var_name, int val );
//void GPmd::query_defined( const char *var_name, int val )
//{
//  char str[256];
//  char *tstr = query( var_name, "" );
//
//  while( sscanf( tstr, "%s", str )>0 )
//  {
//    if( atoi(str) == val )
//      return;
//    tstr+=strlen( str )+1;
//  };
//
//  printf( "[Error] GPmd::query_defined, tag %s %i was not defined.\n", var_name, val );
//  exit(-1);
//}
//
//void load( int num_of_line = 256 );
//void GPmd::load( int num_of_line )
//{
//  for( int i=0; i<n_line; i++ )
//    SAFE_FREE( line[i] );
//    SAFE_FREE( line );
//
//  line = (char**) malloc( n_line * sizeof(char*) );
//}
