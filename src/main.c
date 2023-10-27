#include <stdbool.h>
#include "sqio.h"
#include "tests.h"
#include "squicel.h"

int main(int argc, char *argv[])
{
  const bool tests = true;
  if (tests)
  {
    run_tests();
  }
  else
  {
    squicel();
  }
  return 0;
}