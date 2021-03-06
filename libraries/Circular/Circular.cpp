#include "Circular.h"

//returns true if character written, false 
//if character could not be written
bool Circular::fill(char in) {
  if(!isFull()) {
    buf[head]=in;
    head=(head+1)%N;
    return true;
  }
  //if buffer is full, throw away all unmarked data
  head=mid;
  if(!fullState) bufOverflow++; //wasn't full when we got here, count this as an overflow
  fullState=true;
  return false;
}

char Circular::get() {
  if(isEmpty()) return 0;
  char result=buf[tail];
  tail=(tail+1)%N;
  return result;
}

bool Circular::fill(const char* in) {
  while(*in) {
    if(!fill(*in)) return false;
    in++;
  }
  return true;
}

bool Circular::fill(const char* in, uint32_t len) {
  for(uint32_t i=0;i<len;i++) {
    if(!fill(in[i])) return false;
  }
  return true;
}

char Circular::peekMid(int pos) {
  pos+=mid;
  pos%=N;
  return buf[pos];
}

char Circular::peekHead(int pos) {
  pos+=head;
  pos%=N;
  return buf[pos];
}

char Circular::peekTail(int pos) {
  pos+=tail;
  pos%=N;
  return buf[pos];
}

void Circular::pokeMid(int pos, char poke) {
  pos+=mid;
  pos%=N;
  buf[pos]=poke;
}

void Circular::pokeHead(int pos, char poke) {
  pos+=head;
  pos%=N;
  buf[pos]=poke;
}

void Circular::pokeTail(int pos, char poke) {
  pos+=tail;
  pos%=N;
  buf[pos]=poke;
}

int Circular::unreadylen() {
  int h=head;
  int m=mid;
  if(h<m) h+=N;
  return h-m;
}

int Circular::readylen() {
  int m=mid;
  int t=tail;
  if(m<t) m+=N;
  return m-t;
}

bool Circular::drain(Circular& to) {
  while(readylen()>0) {
    if(to.isFull()) {
      return false;
    }
    fullState=false;
    to.fill(get());
  }
  fullState=false;
  to.mark();
  return true;
}


