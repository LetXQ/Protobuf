#include <iostream>
#include <string.h>
#include <cstdint>
#include <vector>
#include <map>

#include "bytebuffer.h"

void test_buffer()
{
	int8_t a = 10, ta;
	int16_t b = 11, tb;
	int32_t c = 12, tc;
	int64_t d = 13, td;
	uint8_t e = 14, te;
	uint16_t f = 15, tf;
	uint32_t g = 16, tg;
	uint64_t h = 17, th;
	bool i = true, ti;
	bool j = false, tj;
	
 	ByteBuffer buff;
	buff << a << b << c << d << e << f << g << h << i << j;
	
	buff >> ta >> tb >> tc >> td >> te >> tf >> tg >> th >> ti >> tj;
	std::cout << "ta: " << +ta << std::endl;
	std::cout << "tb: " << tb << std::endl;
	std::cout << "tc: " << tc << std::endl;
	std::cout << "td: " << td << std::endl;
	std::cout << "te: " << +te << std::endl;
	std::cout << "tf: " << tf << std::endl;
	std::cout << "tg: " << tg << std::endl;
	std::cout << "th: " << th << std::endl;
	std::cout << "ti: " << (ti ? "True" : "False")<< std::endl;
	std::cout << "tj: " << (tj ? "True" : "False") << std::endl;
	
	std::string src("HelloWorld"), tmp;
	buff.clear();
	buff << src;
	tmp.assign(buff.contents(), buff.size());
	std::string dest;
	buff.clear();
	buff.append(tmp);
	buff >> dest;
	std::cout << "Dest: " << dest << std::endl;
	
	std::vector<int> int_vec, dest_vec;
	int_vec.push_back(1);
	int_vec.push_back(2);
	int_vec.push_back(3);
	int_vec.push_back(4);
	buff.clear();
	buff << int_vec;
	buff >> dest_vec;
	std::cout << "Vector: ";
	for (auto& elem : dest_vec)
	{
		std::cout << elem << " ";
	}
	std::cout << std::endl;
	
	std::map<int, std::string> int_map, dest_map;
	int_map.insert(std::make_pair(1,"hello"));
	int_map.insert(std::make_pair(3, "to"));
	int_map.insert(std::make_pair(2, "nice"));
	int_map.insert(std::make_pair(4, "meet you"));
	buff.clear();
	buff << int_map;
	buff >> dest_map;
	std::cout << "Map: \n";
	for (auto& elem : dest_map)
	{
		std::cout << elem.first << ": " << elem.second << std::endl;
	}
	std::cout << std::endl;
}

int main(void)
{
	test_buffer();
	return 0;
}