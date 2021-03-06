#include "dump.h"

void Dump::region(const char* p, int base, int len, int rec_len) {
  begin();
  while(len>0) {
    if(rec_len>len) rec_len=len;
    line(p,base,rec_len);
    base+=rec_len;
    p+=rec_len;
    len-=rec_len;
  }
  end();
}

void Dump::region(const char* p0, const char *p1, int base, int len0, int len1, int rec_len) {
  begin();
  //Print the first part complete lines
  while(len0>=rec_len) {
    line(p0,base,rec_len);
    base+=rec_len;
    p0+=rec_len;
    len0-=rec_len;
  }
  int len_mid=(rec_len-len0)>len1?len1:(rec_len-len0);
  line(p0,p1,base,len0,len_mid);
  p1+=len_mid;
  len1-=len_mid;
  base+=len_mid+len0;
  while(len1>0) {
    if(rec_len>len1) rec_len=len1;
    line(p1,base,rec_len);
    base+=rec_len;
    p1+=rec_len;
    len1-=rec_len;
  }
  end();
}

void IntelHex::print_byte(unsigned char b) {
  checksum+=b;
  out.print(b>>4,HEX);
  out.print(b & 0x0F,HEX);
}

void IntelHex::begin_line(unsigned char len, unsigned short a, unsigned char type) {
  checksum=0;
  out.print(":");
  print_byte(len);
  print_byte(a>>8);
  print_byte(a & 0xFF);
  print_byte(type);
}

void IntelHex::end_line() {
  print_byte(256-checksum);
  out.println();
}

void IntelHex::begin() {
  addr=0;
}

void IntelHex::end() {
  begin_line(0,0,1);
  end_line();
}

void IntelHex::address(int ia) {
  if((ia & 0xFFFF0000) != (addr & 0xFFFF0000)) {
    addr=ia;
    begin_line(2,0,4);
    print_byte((addr>>24) & 0xFF);
    print_byte((addr>>16) & 0xFF);
    end_line();
  }
}

void IntelHex::line(const char* start, int base, int len) {
  address(base);
  begin_line(len,((unsigned int)base)&0xFFFF,0);
  for(int i=0;i<len;i++) print_byte(start[i]);
  end_line();
}

void IntelHex::line(const char* start0,const char* start1, int base, int len0, int len1) {
  address(base);
  begin_line(len0+len1,((unsigned int)base)&0xFFFF,0);
  for(int i=0;i<len0;i++) print_byte(start0[i]);
  for(int i=0;i<len1;i++) print_byte(start1[i]);
  end_line();
}

void Base85::print_group(const char* p, int len) {
  unsigned int group=0;
  for(int i=0;i<4;i++) {
    group<<=8;
    if(i<len) group |= p[i];
  }
  char group_c[5];
  for(int i=0;i<5;i++) {
    group_c[4-i]=(group % 85)+33;
    group/=85;
  }
  for(int i=0;i<len+1;i++) out.print(group_c[i]);
}

void Base85::line(const char* start, int base, int len) {
  while(len>0) {
    print_group(start,len>4?4:len);
    start+=4;
    len-=4;
  }
  out.println();
}

void Hd::line(const char* start, int base, int len) {
  out.print(base,HEX,4);
  out.print(' ');
  for(int i=0;i<len;i++) {
    out.print(start[i],HEX,2);
    if(i%4==3) out.print(' ');
  }
  for(int i=len;i<preferredLen;i++) {
    out.print("  ");
    if(i%4==3) out.print(' ');
  }
  out.print(' ');
  for(int i=0;i<len;i++) out.print((start[i]>=32 && start[i]<127)?start[i]:'.');
  out.println();
}

void Hd::line(const char* start0, const char* start1, int base, int len0, int len1) {
  out.print(base,HEX,4);
  out.print(' ');
  for(int i=0;i<len0;i++) {
    out.print(start0[i],HEX,2);
    if(i%4==3) out.print(' ');
  }
  for(int i=0;i<len1;i++) {
    out.print(start1[i],HEX,2);
    if((i+len0)%4==3) out.print(' ');
  }
  for(int i=len0+len1;i<preferredLen;i++) {
    out.print("  ");
    if(i%4==3) out.print(' ');
  }
  out.print(' ');
  for(int i=0;i<len0;i++) out.print((start0[i]>=32 && start0[i]<127)?start0[i]:'.');
  for(int i=0;i<len1;i++) out.print((start1[i]>=32 && start1[i]<127)?start1[i]:'.');
  out.println();
}


