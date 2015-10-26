#ifndef BASE_UTILS_CONFIGURE_H
#define BASE_UTILS_CONFIGURE_H
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "../global.h"
#include "../logger.h"
#include <string>

namespace base
{
	namespace utils
	{
		using namespace std;
		using namespace boost::property_tree;
		template<typename T>
		T GetConfigByName(std::string strDir,std::string strKey)
		{
			ptree pt;
			try {
				read_xml(strDir, pt, xml_parser::no_comments);
				return  pt.get_child(strKey).get_value<T>();
				//return  pt.get<T>(strKey);
			} catch (exception& ex) {
				LOG_ERROR("parse gateway configure file fail: %s\n", ex.what());
				return false;
			}
		}

		void test_utils();
	}
}
#endif
