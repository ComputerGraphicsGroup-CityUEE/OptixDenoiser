#ifndef G_MENU_H
#define G_MENU_H

  #define SUB_BEGIN 1124
  #define SUB_END 1125

  typedef struct _G_MENU_INFO
  {
    int  value;
    char label[256];
    char hotkey;
  }G_MENU_INFO;

  const char *parse_char( char ch );
  void parse_menu( G_MENU_INFO *menu_info, int n_menu_info, void (*menu_cmd)(int)  );
  char item2key(int value, G_MENU_INFO *menu_info, int n_menu_info);

#endif