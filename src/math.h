#define PI32 3.14159265359f

// Math types
union v2
{
  struct
  {
    f32 x, y;
  };
  struct
  {
    f32 u, v;
  };
  f32 e[2];
};

union v3
{
  struct
  {
    f32 x, y, z;
  };
  struct
  {
    f32 r, g, b;
  };
  f32 e[3];
};

union v4
{
  struct
  {
    f32 x, y, z, w;
  };
  struct
  {
    f32 r, g, b, a;
  };
  f32 e[4];
};
