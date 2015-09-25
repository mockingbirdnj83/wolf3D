/*
** loading.c for loading in /home/aubess_t/work/oxo/utils
** 
** Made by au
** Login   <aubess_t@epitech.net>
** 
** Started on  Fri Jan 24 21:32:52 2014 au
** Last update Sat Mar  8 12:53:56 2014 au
*/

#define BLACK	"\033[1;30m"
#define RED	"\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW	"\033[1;33m"
#define BLUE	"\033[1;34m"
#define PURPLE	"\033[1;35m"
#define CYAN	"\033[1;36m"
#define GREY	"\033[1;37m"
#define WHITE	"\033[0m"

void	my_usleep()
{
  int	i;
  
  i = 0;
  while (i < 3333333)
    i++;
}

int	main()
{
  int	percent;
  int	equals;
  
  percent = 0;
  equals = 0;
  while (percent <= 100)
    {
      equals = percent / 10;
      my_printf(CYAN);
      my_printf("\r [ %d%% ] [", percent);
      while (equals--)
	my_printf("==");
      equals = percent / 10;
      while (10 - equals++)
	my_printf("  ");
      my_printf("]");
      percent = percent + 1;
      my_usleep();
    }
  my_printf("\n");
  my_printf(WHITE);
  return (0);
}
