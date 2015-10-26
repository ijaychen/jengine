#ifndef BASE_MAP_POINT_H
#define BASE_MAP_POINT_H

#include <boost/unordered_map.hpp>
#include "../gateway/packet_base.h"
namespace base
{
    namespace map
    {
        //格子大小
        static const uint16_t GRID_WIDTH = 25;
        static const uint16_t GRID_HEIGHT = 25;
        //二维坐标
        struct Point {
            Point() : x ( 0 ) , y ( 0 ) {}
            Point ( uint16_t _x , uint16_t _y ) : x ( _x ) , y ( _y ) {}

            uint16_t x;
            uint16_t y;

            uint16_t x_width() const {
                return x * GRID_WIDTH + GRID_WIDTH/2;
            }
            uint16_t y_height() const {
                return y * GRID_HEIGHT + GRID_HEIGHT/2;
            }

            inline bool operator == ( const Point& rhs ) const  {
                return x == rhs.x && y == rhs.y;
            }

            inline bool operator != ( const Point& rhs ) const {
                return x != rhs.x || y != rhs.y;
            }
            // 重载一个 hash_value 函数
            inline friend size_t hash_value ( const Point& p ) {
                size_t seed = 0;
                boost::hash_combine ( seed, boost::hash_value ( p.x ) );
                boost::hash_combine ( seed, boost::hash_value ( p.y ) );
                return seed;
            }

            inline friend base::gateway::PacketOutBase& operator << ( base::gateway::PacketOutBase& pktout , const Point& point ) {
                pktout << point.x << point.y;
                return pktout;
            }
            inline friend base::gateway::PacketInBase& operator >> ( base::gateway::PacketInBase& pktin , Point& point ) {
                pktin >> point.x >> point.y;
                return pktin;
            }
            friend std::ostream& operator << (std::ostream& os , const Point& p) {
                os << "{" << p.x << "," << p.y << "}";
                return os;
            }
        };
    }
}
#endif // BASE_MAP_POINT_H
