#include <stdio.h>
#include <GL/glut.h>

#include "g_menu.h"

const char *parse_char( char ch )
{
  static char str[16];

  switch( ch )
  {
    case '13':
      sprintf( str, "Enter" );
      break;
    case 20:
      sprintf( str, "Space" );
      break;
    case 27:
      sprintf( str, "Esc" );
      break;
    case '\t':
      sprintf( str, "Tab" );
      break;
    default:
      str[0]=ch; str[1]='\0';
      break;
  };

  return str;
}

void parse_menu( G_MENU_INFO *menu_info, int n_menu_info, void (*menu_cmd)(int)  )
{
  int i;

  int mnu_lst[256];
  int mnu_idx=0;

  char label[256];

  mnu_lst[mnu_idx] = glutCreateMenu(menu_cmd);

  for( i=0; i<n_menu_info; i++ )
  {
    switch( menu_info[i].value )
    {
      case SUB_BEGIN:
        mnu_lst[++mnu_idx] = glutCreateMenu(menu_cmd);
        break;
      case SUB_END:
        glutSetMenu( mnu_lst[mnu_idx-1] );
        glutAddSubMenu( menu_info[i].label, mnu_lst[mnu_idx] );
        mnu_idx--;
        break;
      default:
        sprintf( label, "'%s'  -   %s", parse_char(menu_info[i].hotkey), menu_info[i].label );
        glutAddMenuEntry( label, menu_info[i].value );
        break;
    };
  }
}

char item2key(int value, G_MENU_INFO *menu_info, int n_menu_info)
{
  int i;
  for( i=0; i<n_menu_info; i++ )
  {
    if( menu_info[i].value == value )
      return menu_info[i].hotkey;
  }
  return 0;
}
