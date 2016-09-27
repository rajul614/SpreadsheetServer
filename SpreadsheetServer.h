/*
 * Filename:SpreadsheetServer.h
 * Author:Ty-Rack-US Park
 * Last Modified: 4/8/2015
 * Version 1.0
 */

#ifndef SPREADSHEETSERVER_H
#define SPREADSHEETSERVER_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <vector>
#include <set>
#include "Spreadsheet.h"
#include <algorithm>
#include <thread>
#include <mutex>

class SpreadsheetServer
{
 public:
  SpreadsheetServer(); // Default constructor
  SpreadsheetServer(int port); // Builds the server with specified port
  ~SpreadsheetServer(); // Destructor

  void start(); // Initializes the server and begins listening for connections
  void shutdown(int server_socket); 

  std::map<int, const char*> sprd_connections; // Maps client to a filename of a spreadsheet
  std::vector<Spreadsheet*> open_spreadsheets; // List of all active/open spreadsheets
  std::set<std::string> registered_users; // List of all users registered on the server



 private:

  void messageReceived(int client_socket); // This is used to determine what message is sent
  void connectReceived(int client_socket, std::vector<std::string> tokens); // When a connect message is sent this handles that
  void registerReceived(int client_socket, std::vector<std::string> tokens); // Handles register messages which register user
  void cellReceived(int client_socket, std::vector<std::string> tokens); // Handles cell commands to the server finds spreadsheet and changes spreadsheet
  void undoReceived(int client_socket, std::vector<std::string> tokens); // Handles undo messages
  void openSpreadsheet(int client_socket, std::string filename); // helper method used to open a spreadsheet

  void sendConnected(int client_socket, int numcells); // Sends connect message to client
  void sendError(int client_socket, int error_num, std::string info); // Sends error message to client
  void sendCell(int client_socket, std::string cell_name, std::string contents); // Sends cell message to client

  bool save_users(); // Update username file
  bool load_users(); // Load registered usernames from file

  int server_socket; // TCP socket for server
  struct sockaddr_in server_addr; // Address and port for server socket
  std::mutex connections_lock, spreadsheets_lock, users_lock; // Locks for data structures

};

#endif
