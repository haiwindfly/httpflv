#include <http_conn.h>
#include <your_servlet.h>
SERVLET_FUNC  servlet[MAX_TYPE];
unordered_map<string, SERVLET_TYPE> servlet_index_map;
oal_uint8 servlet_register(string name, SERVLET_TYPE n, SERVLET_FUNC func)
{
	if (n < MAX_TYPE)
	{
		servlet_index_map.insert(pair<string, SERVLET_TYPE>(name, n));
		servlet[n] = func;
		return 0;
	}
	else
	{
		return 1;
	}
}
oal_void register_all_servlet()
{
	servlet_register("testservlet",TEST, test_servlet);
}