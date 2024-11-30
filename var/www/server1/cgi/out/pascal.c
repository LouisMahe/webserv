#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int** generate(int numRows, int* returnSize, int** returnColumnSizes)
{
	int	**triangle;
	*returnSize = numRows;
	int	n;
	int	t;
	int	i;

	triangle = malloc(numRows * sizeof(int*));
	*returnColumnSizes = malloc(numRows * sizeof(int));
	triangle[0] = malloc(sizeof(int));
	triangle[0][0] = 1;
	*returnColumnSizes[0] = 1;
	n = 1;
	while (n < numRows)
	{
		t = n + 1;
		triangle[n] = malloc(t*sizeof(int*));
		(*returnColumnSizes)[n] = t;
		triangle[n][0] = 1;
		triangle[n][t - 1] = 1;
		i = 1;
		while (i < t - 1)
		{
			triangle[n][i] = triangle[n - 1][i - 1] + triangle[n - 1][i];
			i++;
		}
		n++;
	}
	return (triangle);
}


int	main(int argc, char **argv, char **env)
{
	(void)argc;
	(void)argv;
	(void)env;
	int	*columnSizes;
	int	trSize;
	char	*query;
	query = getenv("QUERY_STRING");
	int	i = 0;
	while (query[i] != '=')
		i++;
	i++;
	int	len = atoi(&query[i]);

	if (len <= 0)
		len = 2;
	int	**triangle = generate(len, &trSize, &columnSizes);


	printf("Content-Type: text/html\r\n\r\n");

	printf("<html><head><title> HOO le beau triangle </title></head>");
	printf("<body>");
	printf("<style>#one{color:darkred;text-align:center;font-size:300%%;}</style>");
	for (int i = 0; i < len; i++){
		printf("<p id = \"one\">");
		for (int j = 0; j < columnSizes[i]; j++)
			printf("%d ", triangle[i][j]);
		printf("</p>");
	}
	printf("</p></body></html>");

	

}
