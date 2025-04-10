#include <iostream>
#include <string>
#include <fstream>

#include <boost/asio.hpp>  // Критически важно!
#include <boost/system/error_code.hpp>
#include <boost/bind/bind.hpp>

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

#include <filesystem>
#include <queue>
#include <variant>

#include "../Common/Packet.h"