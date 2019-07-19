#include "sem.cpp"
#include <queue>
struct Response{
  string serverResp;
  int requestID;
  int count;
  Response(string str, int req, int ct){
    serverResp = str;
    requestID = req;
    count = ct;
  }
};
class Bounded_buffer{
  int size;
  sem * lock, * full, * empty;
  queue<Response> q;
public:
  Bounded_buffer(int _size){
    size = _size;
    lock = new sem(1);
    full = new sem(0);
    empty = new sem(_size);
  }
  ~Bounded_buffer(){
    delete lock;
    delete full;
    delete empty;
  }
  void addResponse(Response r){
    empty->p();
    lock->p();
    q.push(r);
    lock->v();
    full->v();
  }
  Response getResponse(){
    full->p();
    lock->p();
    Response r = q.front();
    q.pop();
    lock->v();
    empty->v();
    return r;
  }
};
