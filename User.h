
#ifndef USER_H
#define USER_H

#include <string.h>


class User
{
 public:
  User();
  User(char* name, int sock);
  char* username;
  int socket;

};

#endif
