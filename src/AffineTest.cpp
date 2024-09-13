// AffineTest.cpp
// cs200 1/19

#include <cassert>
#include <cmath>
#include <glm/exponential.hpp>
#include <iomanip>
#include <iostream>
#include <string>
#include "Affine.h"
using namespace std;

ostream &operator<<(ostream &s, const glm::vec4 &v) {
  if (cs200::isPoint(v)) return s << '(' << v.x << ',' << v.y << ')';
  if (cs200::isVector(v)) return s << '<' << v.x << ',' << v.y << '>';
  return s << '[' << v.x << ',' << v.y << ',' << v.z << ',' << v.w << ']';
}

ostream &operator<<(ostream &s, const glm::mat4 &A) {
  if (cs200::isAffine(A)) {
    s << "linear={{" << A[0][0] << ',' << A[1][0] << "},{" << A[0][1] << ',' << A[1][1] << "}}, " << "trans=<"
      << A[3][0] << ',' << A[3][1] << '>';
  } else {
    s << '{';
    for (int i = 0; i < 4; ++i) {
      s << '{';
      for (int j = 0; j < 4; ++j) s << A[j][i] << ((j < 3) ? ',' : '}');
      s << ((i < 3) ? ',' : '}');
    }
  }
  return s;
}

int main() {
  const glm::vec4 O = cs200::point(0, 0);
  const float PI = 4.0f * atan(1.0f);
  cout << fixed << setprecision(2);

  glm::vec4 v = cs200::vector(4, -5);
  glm::mat4 A = cs200::translate(v) * cs200::scale(3);
  cout << A << endl;

  glm::vec4 P = cs200::point(9, -2);
  glm::vec4 Q = O + 3.0f * (P - O) + v, R = A * P;
  cout << glm::length(R - Q) << endl;

  A = cs200::rotate(90) * cs200::translate(cs200::vector(7, 5));
  cout << A << endl;

  P = cs200::point(1, 4);
  glm::mat4 B = cs200::translate(P - O) * cs200::rotate(270) * cs200::translate(O - P);
  cout << B << endl;

  Q = cs200::point(5, 8);
  glm::vec4 u = Q - P;
  cout << glm::dot(u, B * Q - P) << endl;

  B = cs200::affine(cs200::vector(5, 2), cs200::vector(1, 4), cs200::point(4, 4));
  cout << B << endl;

  cout << (3.0f * P + 2.0f * Q - 5.0f * R) << endl;
  cout << (4.0f * P - 2.0f * Q - R) << endl;
  cout << (3.0f * P + 2.0f * Q - 7.0f * R) << endl;

  return 0;
}
