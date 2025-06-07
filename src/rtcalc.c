#include <stdio.h>
#include <stdlib.h>

void print_val(int v)

{
  printf("%d\n", v);
}

int read_var(char *s)
{
  char buf[64];
  int val;
  printf("Enter a value for %s: ", s);
  fgets(buf, sizeof(buf), stdin);
  if (EOF == sscanf(buf, "%d", &val))
  {
    printf("Value %s is invalid\n", buf);
    exit(1);
  }
  return val;
}

void print_str(char* s)
{
	printf("%s",s);
}
