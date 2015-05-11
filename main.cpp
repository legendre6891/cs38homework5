#include <limits>
#include <iostream>
#include <cmath>
#include <vector>
#include <queue>
#include <thread>
#include "concurrentqueue.h"
#include <mutex>
#include <string>

using std::cout;
using std::cin;
using std::cos;
using std::modf;
using std::endl;
using std::vector;
using std::fmax;
using std::queue;
using std::thread;
using std::ref;
using std::mutex;
using std::string;

using moodycamel::ConcurrentQueue;

//-------------------------
struct vertex_ {
  double z;
  double w;
  unsigned int l;
};
typedef struct vertex_ vertex;

static const vertex initial = {0.25, -2.0, 2};

static const int nr_threads = 7;

vector<vertex> rets;
vector<vertex> nexts;

static std::mutex barrier;

//-------------------------
inline bool operator<(const vertex& x, const vertex& y) {return x.w < y.w;}
inline bool operator> (const vertex& lhs, const vertex& rhs){return rhs < lhs;}
inline bool operator<=(const vertex& lhs, const vertex& rhs){return !(lhs > rhs);}
inline bool operator>=(const vertex& lhs, const vertex& rhs){return !(lhs < rhs);}
inline bool operator==(const vertex& lhs, const vertex& rhs){return lhs.w == rhs.w;}
inline bool operator!=(const vertex& lhs, const vertex& rhs){return !(lhs == rhs);}


inline double fractional(double z)
{
  double i;
  return modf(z, &i);
}

inline vertex left(vertex v)
{
  double z_new = fractional(5.0 * v.z)/2.0;
  double w_new = v.w - 1.0 + cos(2*M_PI*z_new);
  double l_new = v.l + 1;

  vertex v_new;
  v_new.z = z_new;
  v_new.w = w_new;
  v_new.l = l_new;
  return v_new;
}

inline vertex right_from_left(vertex v0, vertex vleft)
{
  double z_new = vleft.z + 0.5;
  double w_new = v0.w - 1.0 + cos(2*M_PI*z_new);
  double l_new = v0.l + 1;

  vertex v_new;
  v_new.z = z_new;
  v_new.w = w_new;
  v_new.l = l_new;
  return v_new;
}

vertex max_neighbor(vertex v)
{
  vertex vl = left(v);
  vertex vr = right_from_left(v, vl);
  if (vl > vr)
    return vl;
  else
    return vr;
}


double weight_transform(vertex v)
{
  double K = v.w;
  double L = v.l;
  return (K+(L-1.0))/L;
}





//---------------------------------------------------------
void worker(ConcurrentQueue<int> &Q, int j)
{

  Q.enqueue(j);
  return;
}

void worker2(int L, vertex &ret, ConcurrentQueue<vertex> &Q, vertex& next_bound, int id)
{
  vertex v;
  vertex ret2 = ret;
  vertex next_bound2 = next_bound;
  
  while(Q.try_dequeue(v))
    {
      if (v.l == L)
        {
          if (v > ret2)
            ret2 = v;

          vertex t = max_neighbor(v);
          if (t > next_bound2)
            next_bound2 = t;
        }
      vertex vleft = left(v);
      vertex vright = right_from_left(v, vleft);
      if (vleft > ret2)
        Q.enqueue(vleft);
      if (vright > ret2)
        Q.enqueue(vright);
    }
  rets[id] = ret2;
  nexts[id] = next_bound2;
  return;
}



vertex find_with_lower_bound(int L, vertex v_bound, vertex& next_bound)
{
  vertex ret = v_bound;
  next_bound = max_neighbor(ret);
  
  ConcurrentQueue<vertex> Q;
  Q.enqueue(initial);

  bool flag = false;

  while(Q.size_approx() < 10000)
    {
      vertex v;
      flag = Q.try_dequeue(v);
      if (flag == false)
        break;

      if (v.l == L)
        {
          if (v > ret)
            ret = v;

          vertex t = max_neighbor(v);
          if (t > next_bound)
            next_bound = t;
        }
      else
        {
          vertex vleft = left(v);
          vertex vright = right_from_left(v, vleft);
          if (vleft > ret)
              Q.enqueue(vleft);
          if (vright > ret)
              Q.enqueue(vright);
        }
    }
  if (flag == false)
    return ret;
  else
    {
      vector<thread> th;

      for (int i = 0; i < nr_threads; i++)
        th.push_back(thread(worker2, L, ref(ret), ref(Q), ref(next_bound), i));

      for(auto &t : th)
        t.join();

      ret = rets[0];
      next_bound = nexts[0];

      for(auto v : rets)
        if (v > ret)
          ret = v;

      for(auto v: nexts)
        if (v > next_bound)
          next_bound = v;

      return ret;
    }
  
}


//---------------------------------------------------------

int main(int argc, char* argv[])
{
  int Lmax = std::stoi(std::string(argv[1]));
  rets.reserve(nr_threads);
  nexts.reserve(nr_threads);
  
  cout << "L = " << 1 << "    Max = " << -1 << endl;
  cout << "L = " << 2 << "    Max = " << -0.5 << endl;
    vertex vbest = initial;
  vertex vbound = max_neighbor(initial);
  vertex v;
  for (int L = 3; L <= Lmax; L++)
    {
      vbest = find_with_lower_bound(L, vbound, v);
      cout << "L = " << L << "    Max = " << weight_transform(vbest) << endl;
      vbound = v;
    }
  return 0;
}
