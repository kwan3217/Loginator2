#ifndef Circular_h
#define Circular_h

#include <inttypes.h>

//Circular buffer with attempt at atomic message write
//The buffer consists of a block of memory which conceptually contains
//two parts - data which is ready to be flushed, and data which is not
//yet ready. The buffer writer decides when the data which wasn't ready
//becomes so, and when it is, it is added to the data which is ready.
//The buffer gets written to by the generic fill() function. 
//A special drain() function empties data out of the buffer somehow and
//moves the head ptr up.
//All circular buffers are 1024 characters.
class Circular {
protected:
  uint32_t N;
  char* buf;
  //Location of the next slot to be written to
  uint32_t volatile head;
  //Location of the next slot not yet ready to be flushed
  uint32_t volatile mid;
  //Location of the next slot to be flushed
  uint32_t volatile tail;
  //If set, buffer reached full. All unmarked data was tossed and 
  //no new data should be accepted until buffer is drained.
  bool fullState;
  uint32_t bufOverflow;
public:
  Circular(uint32_t LN, char* Lbuf):N(LN),buf(Lbuf),head(0),mid(0),tail(0),fullState(false),bufOverflow(0) {}
  //Is there no space to write another char?  
  bool isFull() {return fullState || ((head+1)%N==tail);};
  //Is there at least one char ready to be read?
  bool isEmpty() {return head==tail;};
  //Add a char to the buffer, not ready to be flushed yet
  bool fill(char in);

  void empty() {head=0;tail=0;mid=0;};

  bool fill(const char* in);
  bool fill(const char* in, uint32_t len);

  //Fill in Big-endian
  bool fill16BE(uint32_t in) {
    if(!fill((char)((in >> 8) & 0xFF))) return false;
    if(!fill((char)((in >> 0) & 0xFF))) return false;
    return true;
  }
  bool fill32BE(uint32_t in) {
    if(!fill((char)((in >> 24) & 0xFF))) return false;
    if(!fill((char)((in >> 16) & 0xFF))) return false;
    if(!fill((char)((in >>  8) & 0xFF))) return false;
    if(!fill((char)((in >>  0) & 0xFF))) return false;
    return true;
  };
  bool fill16LE(uint16_t in) {return fill((char*)&in,2);};
  bool fill32LE(uint32_t in) {return fill((char*)&in,4);};

  //Mark all current unready data as ready
  virtual void mark() {mid=head;};


  //Get the next character ready to be flushed
  char get();
  //Get all ready data from one buffer and copy it to another (as ready also)
  //Returns true if some data was drained, false if not. 
  bool drain(Circular& to);
  //Intended to do a drain to something that a subclass knows about, say 
  //a file or stream printer.
  virtual bool drain() {return true;};
  //Get the number of characters which aren't ready yet
  int unreadylen();
  //Get the number of characters which are ready
  int readylen();

  char peekTail(int ahead=0);
  char peekMid(int ahead=0);
  char peekHead(int ahead=0);

  void pokeTail(int ahead, char poke);
  void pokeMid(int ahead, char poke);
  void pokeHead(int ahead, char poke);

  char* volatile headPtr() {return buf+head;}
  char* volatile tailPtr() {return buf+tail;}
  char* volatile midPtr()  {return buf+mid;}
  uint32_t getBufOverflow() {return bufOverflow;}
};

#endif

