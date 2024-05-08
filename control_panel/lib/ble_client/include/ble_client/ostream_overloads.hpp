#pragma once

#include <iostream>

#include <simpleble/SimpleBLE.h>

std::ostream& operator<<(std::ostream& os, SimpleBLE::Peripheral& peripheral);
std::ostream& operator<<(std::ostream& os, SimpleBLE::Adapter& adapter);
std::ostream& operator<<(std::ostream& os, SimpleBLE::Service& service);
std::ostream& operator<<(std::ostream& os, SimpleBLE::Characteristic& characteristic);
std::ostream& operator<<(std::ostream& os, SimpleBLE::Descriptor& descriptor);
std::ostream& operator<<(std::ostream& os, std::vector<SimpleBLE::Adapter> &adapters);