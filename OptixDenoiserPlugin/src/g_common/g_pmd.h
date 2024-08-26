#ifndef G_PMD_H
#define G_PMD_H


class GPmd
{
  public:
    GPmd();
    ~GPmd();

    void load( const char *spath );
    void save( const char *spath );

    int   query_count( const char *var_name );
    char* query_array( const char *var_name, const char *format,  ... );
    char* query_index( const char *var_name, int idx, const char *format,  ... );
    char* query( const char *var_name, const char *format,  ... );
    char* setTag( const char *var_name, const char *format, ... );
    char* soft_query( const char *var_name, const char *format,  ... );
    char* soft_query_bool( const char *var_name, bool *value );

    char* query_path( const char *var_name, char *spath );
    char* query_dir( const char *var_name, char *spath );
    void addline( const char *str );

    char **line;
    int n_line;

  private:
    char* vsquery( const char *var_name, const char *format, char *va );

    char pmd_path[256];

    char qa_tag[16][64];
    int  qa_idx[16];
};





#endif