#include "helpers.h"
#include <string.h>

void	*ft_calloc(size_t nmemb, size_t size)
{
	void	*res;

	if (nmemb == 0 || size == 0)
		return (malloc(1));
	if (nmemb != 0 && size > (size_t)-1 / nmemb)
		return (NULL);
	res = malloc(nmemb * size);
	if (!res)
		return (NULL);
	memset(res, '\0', nmemb * size);
	return (res);
}
