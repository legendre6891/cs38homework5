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

vertex lower_bound(unsigned int L)
{
  vertex ret = {0.0, -INFINITY, 0};
  vector<bool> vec;
  vec.reserve(L-2);
  for(int i = 0; i < 100000; i++)
    {
      vec.clear();
      for(int j = 0; j < L-2; j++)
        {
          vec.push_back(d(gen));
        }
      vertex nominal = follow_sequence(vec);
      if (nominal > ret)
        ret = nominal;
    }
  return ret;
}

double weight_transform(vertex v)
{
  double K = v.w;
  double L = v.l;
  return (K+(L-1.0))/L;
}

vertex find_with_lower_bound(int L, vertex v_bound)
{
  vertex ret = v_bound;
  queue<vertex> Q;
  Q.push(initial);

  unsigned int i = 0;
  while(Q.empty() == false)
    {
      vertex v = Q.front();
      Q.pop();

      i++;

      if (i % 100000 == 0)
        {
          // cout << "Q.size = " << Q.size() << "      ";
          // cout << "Level = " << v.l << endl;
        } 

      if (v.l == L)
        {
          if (v > ret)
              ret = v;
        }
      else
        {
          vertex vleft = left(v);
          vertex vright = right_from_left(v, vleft);

          if (vleft > ret)
            Q.push(vleft);
          if (vright > ret)
            Q.push(vright);
        }
    }
  return ret;
}



int main()
{
  int Lmax;
  cout << "Enter a value for Lmax: ";
  cin >> Lmax;

cout << "L = " << 1 << "    Max = " << -1 << endl;
cout << "L = " << 2 << "    Max = " << -0.5 << endl;
  vertex vbest = initial;
  vertex vbound;
  for (int L = 3; L <= Lmax; L++)
    {
      vbound = max_neighbor(vbest);
      vbest = find_with_lower_bound(L, vbound);
      cout << "L = " << L << "    Max = " << weight_transform(vbest) << endl;
      vbound = vbest;
    }
  return 0;
}
