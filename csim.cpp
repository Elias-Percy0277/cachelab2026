// student id: 2025201743
// please change the above line to your student id
#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <list>
#include <unordered_map>
#include <vector>
using namespace std;
int s, E, b;
int hits = 0, misses = 0, evictions = 0;

struct CacheLine
{
  unsigned long tag;
};

struct CacheSet
{
  list<CacheLine> lines;
  unordered_map<unsigned long, std::list<CacheLine>::iterator> line_map;
};

void printSummary(int hits, int misses, int evictions)
{
  printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
  FILE *output_fp = fopen(".csim_results", "w");
  assert(output_fp);
  fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
  fclose(output_fp);
}

void printHelp(const char *name)
{
  printf(
      "Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n"
      "Options:\n"
      "  -h         Print this help message.\n"
      "  -v         Optional verbose flag.\n"
      "  -s <num>   Number of set index bits.\n"
      "  -E <num>   Number of lines per set.\n"
      "  -b <num>   Number of block offset bits.\n"
      "  -t <file>  Trace file.\n\n"
      "Examples:\n"
      "  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n"
      "  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n",
      name, name, name);
}

int main(int argc, char *argv[])
{
  char *tracefile = nullptr;
  bool verbose = false;
  int opt;
  while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1)
  {
    switch (opt)
    {
    case 'h':
      printHelp(argv[0]);
      exit(0);
    case 'v':
      verbose = true;
      break;
    case 's':
      s = atoi(optarg);
      break;
    case 'E':
      E = atoi(optarg);
      break;
    case 'b':
      b = atoi(optarg);
      break;
    case 't':
      tracefile = optarg;
      break;
    default:
      printHelp(argv[0]);
      exit(1);
    }
  }

  if (s < 0 || E <= 0 || b < 0 || tracefile == nullptr)
  {
    printf("Missing required argument\n");
    return 1;
  }

  FILE *fp = fopen(tracefile, "r");
  if (fp == nullptr)
  {
    printf("Error: cannot open %s\n", tracefile);
    return 1;
  }

  char op;
  unsigned long addr;
  int size, reg;
  vector<CacheSet> cache(1 << s);
  while (fscanf(fp, " %c %lx,%d %d", &op, &addr, &size, &reg) == 4)
  {
    unsigned long tag = addr >> (s + b);
    unsigned long set_index = addr >> (b) & ((1 << s) - 1);
    unsigned long block_offset = addr & ((1 << b) - 1);
    CacheSet &cache_set = cache[set_index];
    auto it = cache_set.line_map.find(tag);
    if (it != cache_set.line_map.end())
    {
      hits++;
      cache_set.lines.splice(cache_set.lines.begin(), cache_set.lines, it->second);
      if (verbose)
        printf("%c %lx,%d hit\n", op, addr, size);
    }
    else
    {
      misses++;
      if (cache_set.lines.size() == E)
      {
        evictions++;
        auto last_line = cache_set.lines.back();
        cache_set.line_map.erase(last_line.tag);
        cache_set.lines.pop_back();
        if (verbose)
          printf("%c %lx,%d miss eviction\n", op, addr, size);
      }
      else
      {
        if (verbose)
          printf("%c %lx,%d miss\n", op, addr, size);
      }
      CacheLine new_line{tag};
      cache_set.lines.push_front(new_line);
      cache_set.line_map[tag] = cache_set.lines.begin();
    }
  }
  fclose(fp);
  printSummary(hits, misses, evictions);
  return 0;
}