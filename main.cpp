#include <limits>
#include <iostream>
#include <cmath>
#include <vector>
#include <random>
#include <queue>


using std::cout;
using std::cin;
using std::cos;
using std::modf;
using std::endl;
using std::vector;
using std::fmax;
using std::queue;

//-------------------------
struct vertex_ {
  double z;
  double w;
  unsigned int l;
};

typedef struct vertex_ vertex;

static const vertex initial = {0.25, -2.0, 2};


std::random_device rd;
std::mt19937 gen(rd());
std::bernoulli_distribution d(0.5);

//-------------------------


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

vertex follow_sequence(vector<bool> vec)
{
  vertex v = initial;
  for (bool b : vec)
    {
      vertex vleft = left(v);
      if (b)
        v = left(v);
      else
        v = right_from_left(v, vleft);
    }
  return v;
}

double follow_sequence_weight(vector<bool> vec)
{
  vertex v = follow_sequence(vec);
  return v.w;
}

double lower_bound(unsigned int L)
{
  double H = -INFINITY;
  vector<bool> vec;
  vec.reserve(L-2);
  for(int i = 0; i < 100000; i++)
    {
      vec.clear();
      for(int j = 0; j < L-2; j++)
        {
          vec.push_back(d(gen));
        }
      H = fmax(H, follow_sequence_weight(vec));
    }
  return H;
}

double transform(double K, double L)
{
  return (K+(L-1.0))/L;
}

double find_with_lower_bound(int L, double H)
{
  double K = H;
  queue<vertex> Q;
  Q.push(initial);

  unsigned int i = 0;
  while(Q.empty() == false)
    {
      vertex v = Q.front();
      Q.pop();

      double w = v.w;
      double l = v.l;

      i++;

      if (i % 10000 == 0)
        {
          cout << "Q.size = " << Q.size() << "      ";
          cout << "Level = " << l << endl;
        } 

      if (l == L)
        {
          if (w > K)
            K = w;
        }
      else
        {
          vertex vleft = left(v);
          vertex vright = right_from_left(v, vleft);

          if (vleft.w > K)
            Q.push(vleft);
          if (vright.w > K)
            Q.push(vright);
        }
    }
  return transform(K,L);
}



int main()
{
  int L;
  cout << "Enter a value for L: ";
  cin >> L;

  double H = lower_bound(L);
  cout << "The lower bound is: " << transform(H,L) << endl;
  cout << find_with_lower_bound(L,H) << endl;
}
