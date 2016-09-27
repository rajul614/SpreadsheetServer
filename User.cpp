#include "User.h"

User::User()
{
}
User::User(char* name, int sock)
{
  username = name;
  socket = sock;
}

