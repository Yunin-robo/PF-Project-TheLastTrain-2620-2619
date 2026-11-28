#include "util.h"
#include <iostream>
#include <cstdlib>
using namespace std;

int clampInt( int value, int min_val, int max_val){
  if (value < min_val) { return min_val;}
  if (value > max_val) {return max_val;}
  return value;}

bool stringEquals( const char* p, const char* q){
  if (p==nullptr  || q == nullptr){ return false;}
  
  int i = 0;
  while (p[i] != '\0'  && q[i] != '\0'){ if (p[i]!= q[i]) {return false;} i++;}
  return p[i]=='\0' && q[i] == '\0'; }
  
void trimWhitespace (char* str){
  if (str==nullptr){return;}
  int st = 0;
  int en = 0;
  while (str[en]!='\0'){ en++;}
  while (str[st] == ' ' || str[st] == '\t' || str[st] == '\n' || str[st] == '\r') {st++;}
   en--;
  while (en>= st && (str[en] == ' ' || str[en] == '\t' || str[en] == '\n' || str[en] == '\r')) { en--;}
  int i = 0;
  while (st<=en) { str[i] = str[st]; i++; st++;} 
  str[i] = '\0';}

void printHeader(const char* title) {
cout << endl;
cout << "+==========================================+" << endl;
cout << "|                                          |" << endl;
cout << "              " << title << "              " << endl;
cout << "|                                          |" << endl;
cout << "+==========================================+" << endl;
cout << endl;}

void clearScreen() {
  cout << "\033[2J\033[3J\033[H";
  cout.flush();}

// Primitive helper utilities, clamping, and screen control
