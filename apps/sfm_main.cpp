#include <iostream>

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cerr << "usage: " << argv[0] << " <config.yaml>\n";
    return 1;
  }
  std::cerr << "pipeline not implemented yet (see ROADMAP.md)\n";
  return 1;
}
