/*
 * Filename: SpreadsheetServer.cpp
 * Author: Ty-Rack-US Park
 * Last Modified: 4/26/2015
 * Version 1.0
 */

#include "SpreadsheetServer.h"

using namespace std;
/*
 * Helper method used when the server has hit a dire state. This is used when the server cannot be started or a binding error has occured. 
 */
void error(const char *msg)
{
  perror(msg);
  exit(1);
}

int main(int argc, char *argv[])
{
  if (argc < 2)
    {
      cout << "Please provide a port number for the server" << endl;
      return 0;
    }

  SpreadsheetServer server (atoi(argv[1]));
  server.start();

  return 0;
}


SpreadsheetServer::SpreadsheetServer()
{

}

SpreadsheetServer::SpreadsheetServer(int port)
{
  // Create a TCP socket
  server_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (server_socket < 0) 
    error("ERROR opening socket");

  // Clears server address buffer
  bzero((char *) &server_addr, sizeof(server_addr)); 

  // Sets the values for server address
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  // Bind the socket to the address
  if (::bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    error("ERROR on binding");

  // Load users on server
  if(!load_users())
    {
      registered_users.insert("sysadmin");
      save_users();
    }
  else
    {
      cout<<"Loaded users successfully"<<endl;
    }

  cout << "Server initialized" << endl;
}

SpreadsheetServer::~SpreadsheetServer()
{
}

/*
 * This method starts the server and begins listening for connections from clients
 *
 */
void SpreadsheetServer::start()
{
  socklen_t client_length;
  struct sockaddr_in client_addr;
  int client_socket, pid;

  // Listen for connections on the server socket
  listen(server_socket, 5); 
  client_length = sizeof(client_addr);

  cout << "Server listening" << endl;

  std::thread (&SpreadsheetServer::shutdown, this, server_socket).detach();

  // Accepts connections from clients and send their execution on separate threads
  while (1)
    {
      client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_length); 

      if (client_socket < 0)  
	error("ERROR on accept");

      std::thread (&SpreadsheetServer::messageReceived, this, client_socket).detach();
    }

  // Close the server socket
  close(server_socket);
}

void SpreadsheetServer::shutdown(int server_socket)
{
  std::string input;
  cin >> input;
  if (input.compare("QUIT") == 0)
    {
      close(server_socket);
      cout << "Server shutting down" << endl;
      exit(0);
    }
  else
    shutdown(server_socket);
}

/* 
 * This makes a text file from the registered user data structure. The text file is used to store the registered users on the server
 */
bool SpreadsheetServer::save_users()
{
  ofstream userfile ("usernames.usrs");
  if(userfile.is_open()){
    for (std::set<string>::iterator it=registered_users.begin(); it!=registered_users.end(); ++it) 
      {
	userfile << *it+ "\n"; // Seperates all users with a \n
      }
    userfile.close();
    return true;
  }
  else {
    // File didn't open
    cout << "Unable to open user file" << endl;
    return false;
  }
  
}

/*
 * At the start of the server this method is called to grab the users text file from the system and register all users on that file.
 * Return true if a file was found
 * Return fasle if a file was not found
 */
bool SpreadsheetServer::load_users()
{
  string line;

  // Opens the file
  ifstream userfile ("usernames.usrs");

  // If the was opened/exists
  if (userfile.is_open())
    {
      // Read in the useranmes and insert into our data structure
      while (getline (userfile,line) )
	{
	  std::string username;
	  stringstream ss(line);
	  
	  ss>>username;
	  registered_users.insert(username);
	  
	}
      userfile.close();
      return true;
    }
  else
    {
      cout<<"Username file does not exist"<<endl;
      return false;
    }
}

/*
 * Listens for messages, parses the messages, and calls the specific functions to handle commands
 * Handles disconnects by clients
 *
 */
void SpreadsheetServer::messageReceived(int client_socket)
{
  std::string line, next;
  char buffer[1024];
  int n = 0;
  std::vector<std::string> tokens;

  // Read one byte at a time from client
  n = read(client_socket, buffer, 1);

  // Bytes were read
  while (n > 0)
    {
      // Read until a '\n' character
      char c = buffer[0];
      if (c == '\n')
	break;

      // Append the character to the line, which contains the message
      else
	line.append(1, c);

      // Continue reading until end of message
      n = read(client_socket, buffer, 1);
    }
  
  // If an error occured or client disconnected
  if (n <= 0)
    {
       cout << "client disconnected" << endl;

       // Remove connection of client and their spreadsheet
       connections_lock.lock();
       const char * filename = sprd_connections.find(client_socket)->second;
       sprd_connections.erase(client_socket);
       connections_lock.unlock();

       spreadsheets_lock.lock();

       // Find the spreadsheet they were working on
       for (int i = 0; i < open_spreadsheets.size(); i++)
	 {
	   // Compare by filename
	   if (strcmp(open_spreadsheets.at(i)->filename, filename) == 0)
	     {
	       // Remove the socket from the spreadsheet
	       Spreadsheet * s = open_spreadsheets.at(i);
	       s->sockets.erase(find(s->sockets.begin(), s->sockets.end(), client_socket));

	       // If there are no more connections to the spreadsheet
	       // It is considered inactive and no longer open
	       if (s->sockets.size() == 0)
	       {
	         delete s;
	         open_spreadsheets.erase(open_spreadsheets.begin()+i);
	       }
	     }
	 }
       spreadsheets_lock.unlock();

       return;
    }

  // Identify which client sent the message in console
  cout << "Client " << client_socket << ": " << line << endl;

  // Split the message by whitespace
  std::stringstream ss(line);
  std::string token;

  while (ss >> token)
    tokens.push_back(token);
  
  // Handles case when client only sends an enter command in telnet
  // i.e. "\r\n"
  std::string command;
  if (tokens.size() > 0)
    command = (tokens.at(0));
  else
    command = "";

  // Connect command
  if (command.compare("connect") == 0)     
    connectReceived(client_socket, tokens);

  // Regsiter command
  else if (command.compare("register") == 0)    
    registerReceived(client_socket, tokens);
    
  // Cell command
  else if (command.compare("cell") == 0)    
    cellReceived(client_socket, tokens);
    
  // Undo command
  else if (command.compare("undo") == 0)    
    undoReceived(client_socket, tokens);
    
  // Unrecognized command
  else    
    sendError(client_socket, 2, "Invalid command");
    
  // Listen for more messages
  messageReceived(client_socket);
}

/*
 * Connect command received
 * Checks to see if client is already connected to a spreadsheet: sends error message in that case
 * Checks for syntax in the command: sends appropriate error message
 * Checks to see if username is registered: error message if not registered
 * If everything is correct, open the spreadsheet requested
 */
void SpreadsheetServer::connectReceived(int client_socket, std::vector<std::string> tokens)
{
  std::string user, filename;

  // Check to see if client has connected
  connections_lock.lock();
  int con = sprd_connections.count(client_socket);
  connections_lock.unlock();

  // Already connected
  if (con == 1)
    {
      sendError(client_socket, 3, "Already connected to a spreadsheet");
      return;
    }

  // Too many or too few arguments
  if (tokens.size() != 3)
    {
      sendError(client_socket, 2, "Incorrect number of tokens");
      return;
    }

  user = tokens.at(1);
  filename = tokens.at(2);

  // Check to see if user has been registered
  bool registered = false;
  users_lock.lock();
  registered = registered_users.find(user) != registered_users.end();
  users_lock.unlock();

  // Not registered
  if (!registered)
      sendError(client_socket, 4, "Username is not registered or is taken");

  // Registered
  else
    openSpreadsheet(client_socket, filename);

}

/*
 * Called when a client sends a valid connect message
 * Finds the spreadsheet that the client requested, either by creating one or loading one
 * Sends the client the connected message and all the cells to load the spreadsheet
 */
void SpreadsheetServer::openSpreadsheet(int client_socket, std::string filename)
{
  const char* file = filename.c_str();
  bool exists = false;
  int numcells;
  Spreadsheet * s;

  // Lock the data structure
  spreadsheets_lock.lock();

  // See if the spreadsheet is already opened
  for (int i = 0; i < open_spreadsheets.size(); i++)
    {
      // If the spreadsheet is in the list
      if (strcmp(open_spreadsheets.at(i)->filename, file) == 0)
	{
	  exists = true;
	  s = open_spreadsheets.at(i);
	  break;
	}
    }

  // If the spreadsheet has not been opened yet
  if (!exists)
    {      
      // Add to open spreadsheets
      open_spreadsheets.push_back(new Spreadsheet(file));
      int back = open_spreadsheets.size()-1;
      s = open_spreadsheets.at(back);
    }
  spreadsheets_lock.unlock();

  // Lock the individual spreadsheet
  s->lock.lock();

  // Add the socket to the spreadsheet
  s->sockets.push_back(client_socket);

  // Send the connected command with the number of cells to load
  numcells = s->cells.size();
  sendConnected(client_socket, numcells);

  // Iterate through all the cells and send them to the client
  for (map<string, string>::iterator it = s->cells.begin(); it != s->cells.end(); it++)
    {
      string cell = it->first;
      string content = it->second;
	      
      sendCell(client_socket, cell, content);
    }
  s->lock.unlock();

  // Establish the connection between socket and the spreadsheet
  connections_lock.lock();
  sprd_connections.insert(std::pair<int, const char*>(client_socket, file));
  connections_lock.unlock();

}

/*
 * Register command received
 * Checks to see if client has connected to a spreadsheet
 * Checks for syntax in the command: sends appropriate error message for incorrect syntax
 * Registers the user and saves it to file
 * If user was already registered, send back an error message notifying client
 */
void SpreadsheetServer::registerReceived(int client_socket, std::vector<std::string> tokens)
{
  std::string username;

  // Check to see if client has connected
  spreadsheets_lock.lock();
  int con = sprd_connections.count(client_socket);
  spreadsheets_lock.unlock();

  // Not connected
  if (con != 1)
    {
      sendError(client_socket, 3, "Cannot perform command before connecting to a spreadsheet");
      return;
    }

  // Only sent the register command
  if (tokens.size() == 1)
    {
      sendError(client_socket, 2, "Incorrect number of tokens");
      return;
    }

  // Sent more than register command or name had spaces
  else if (tokens.size() > 2)
    {
      sendError(client_socket, 4, "Invalid username");
      return;
    }

  username = tokens.at(1);

  // Register the user and save to file
  users_lock.lock();
  std::pair<std::set<std::string>::iterator, bool> ret = registered_users.insert(username);
  save_users();
  users_lock.unlock();

  // User was already registered
  if (ret.second == false)
    sendError(client_socket, 4, "Username already registered");

}

/*
 * Cell command received
 * Checks that client has connected to a spreadsheet
 * Checks for syntax: sends an error message if syntax is incorrect
 * Makes the changes to the corresponding spreadsheet
 * Sends back the changes to all clients on the spreadsheet or
 * Sends back a circular dependency error message to the client that caused it
 */
void SpreadsheetServer::cellReceived(int client_socket, std::vector<std::string> tokens)
{
  std::string cell, contents;

  // Check to see if client has connected
  connections_lock.lock();
  int con = sprd_connections.count(client_socket);
  connections_lock.unlock();

  // Not connected
  if (con != 1)
    {
      sendError(client_socket, 3, "Cannot perform command before connecting to a spreadsheet");
      return;
    }

  // Only sent 'cell\n'
  if (tokens.size() == 1)
    {
      sendError(client_socket, 2, "Incorrect number of tokens");
      return;
    }

  // Setting contents of a cell to ""
  // "cell A1\n"
  else if (tokens.size() == 2)
    contents = "";

  // Setting contents of a cell regularly
  // Notice, contents can contain spaces
  else
    {
      // Grab all the contents from the command
      for (int i = 2; i < tokens.size(); i++)
	contents += tokens.at(i) + " ";

      // Remove excess " " at the end
      contents = contents.substr(0, contents.size()-1);
    }

  cell = tokens.at(1);

  // Check to see that the cell name is syntactically correct
  int chars = cell.size();
  char c = cell[0];

  // Cell name should have either 2 or 3 characters
  if (cell.size() != 2 && cell.size() != 3)
    {
      sendError(client_socket, 2 ,"Invalid cell name");
      return;
    }

  // Does not start with a letter
  if (!(c >= 65 && c <= 90) && !(c >= 97 && c <= 122))
    {
      sendError(client_socket, 2 ,"Invalid cell name");
      return;
    }   

  // Check the remaining characters to see if they are numbers
  for (int i = 1; i < chars; i++)
    {
      // First character cannot be a 0
      if (i == 1)
	{
	  if (!(cell[i] >= 49 && cell[i] <= 57))
	    {
	      sendError(client_socket, 2, "Invalid cell name");
	      return;
	    }
	}

      // Second character can be any digit
      else if (!(cell[i] >= 48 && cell[i] <= 57))
	{
	  sendError(client_socket, 2, "Invalid cell name");
	  return;
	}
    }
  
  // Find spreadsheet associated with socket
  connections_lock.lock();
  const char * filename = sprd_connections.find(client_socket)->second;
  connections_lock.unlock();

  Spreadsheet * s;

  // Find the spreadsheet
  spreadsheets_lock.lock();

  for (int i = 0; i < open_spreadsheets.size(); i++)
    {
      if (strcmp(filename, open_spreadsheets.at(i)->filename) == 0)
	{
	  s = open_spreadsheets.at(i);	 
	  break;
	}
    }
  spreadsheets_lock.unlock();

  // Attempt to make the change to the spreadsheet
  s->lock.lock();
  bool not_cir = s->setCell(cell, contents);

  // Successfully changed contents
  if (not_cir)
    {	
      // Send the change to all sockets on the spreadsheet
      vector<int> sockets = s->sockets;          
      for(int i = 0; i < sockets.size(); i++)
	{
	  sendCell(sockets.at(i), cell, contents);
	}    
    }

  // Circular dependency occurs
  else
    sendError(client_socket, 1, "Circular dependency occurs with this change");

  s->lock.unlock();

}

/*
 * Undo message received
 * Sends undo command to corresponding spreadsheet
 * Sends changes back to clients or
 * Sends back error message if undo could not happen
 */
void SpreadsheetServer::undoReceived(int client_socket, std::vector<std::string> tokens)
{
  // Check to see client has connected
  connections_lock.lock();
  int con = sprd_connections.count(client_socket);
  connections_lock.unlock();

  // Not connected
  if (con != 1)
    {
      sendError(client_socket, 3, "Cannot perform command before connecting to a spreadsheet");
      return;
    }

  // More than just "undo\n"
  if (tokens.size() != 1)
    {
      sendError(client_socket, 2, "Incorrect number of tokens");
      return;
    }

  // Find the spreadsheet associated with the socket
  connections_lock.lock();
  const char* filename = sprd_connections.find(client_socket)->second;
  connections_lock.unlock();

  // Find the spreadsheet
  Spreadsheet * s;

  spreadsheets_lock.lock();
  for (int i = 0; i < open_spreadsheets.size(); i++)
    {
      if (strcmp(filename, open_spreadsheets.at(i)->filename) == 0)
	{
	  s = open_spreadsheets.at(i);	 
	  break;	
	}
    }
  spreadsheets_lock.unlock();
  
  // Call undo on the spreadsheet
  s->lock.lock();
  std::pair<std::string, std::string> change;
  change = s->undo();

  // Get the change from undoing
  string name, contents;
  name = change.first;
  contents = change.second;

  // Undo successful
  if (name.compare("ERROR") != 0)
    {
      // Send the resulting changes to all sockets on the spreadsheet
      vector<int> sockets = s->sockets;
      for (int i = 0; i < sockets.size(); i++)
	sendCell(sockets.at(i), name, contents);
    }

  // Could not undo because no changes have been made to the spreadsheet
  else
    sendError(client_socket, 3, "Cannot undo before a change has been made");

  s->lock.unlock();
}

/*
 * Constructs the connect message and sends it to the client
 */
void SpreadsheetServer::sendConnected(int client_socket, int numcells)
{
  int length, n;
  char message[256];

  // Builds the message
  length = sprintf(message, "connected %d\n", numcells);

  if (length < 0)
    error("ERROR creating connected message");

  // Send the message
  n = write(client_socket, message, length);
  cout << "Server to " << client_socket << ": " << message;

  if (n < 0)
    error("ERROR writing to socket");
}

/*
 * Construcs the cell message and sends it to the client
 */
void SpreadsheetServer::sendCell(int client_socket, std::string cell_name, std::string contents)
{
  int length, n;
  const char * cell = cell_name.c_str();
  const char * content = contents.c_str();
  char message[256];

  // Build the message
  length = sprintf(message, "cell %s %s\n", cell, content);

  if (length < 0)
    error("ERROR creating send cell message");

  // Send the message
  n = write(client_socket, message, length);
  cout << "Server to " << client_socket << ": " << message;

  if (n < 0)
    error("ERROR writing to socket");
}

/*
 * Constructs the error message and sends it to the client
 */
void SpreadsheetServer::sendError(int client_socket, int error_num, std::string info)
{
  int n, length;
  const char * err = info.c_str();
  char message[256];

  // Build the message
  length = sprintf(message, "error %d %s\n", error_num, err);

  if (length < 0)
    error("ERROR creating client error message");

  // Send the message
  n = write(client_socket, message, length);
  cout << "Server to " << client_socket << ": " << message;

  if (n < 0)
    error("ERROR writing to socket");

}
