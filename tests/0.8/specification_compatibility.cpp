#include "nysor/block_loader.hpp"

#include <iostream>
#include <stdexcept>

int main() {
  try {
    const nysor::BlockDefinition producer{
        "Timer", "1.0.0", "source", "time", 0, 1, {},
        {{"event", "core.event", "core.event"}}};
    const nysor::BlockDefinition event_consumer{
        "Counter", "1.0.0", "unary", "event", 1, 1,
        {{"trigger", "core.event", "core.event"}},
        {{"fired", "core.event", "core.event"}}};
    const nysor::BlockDefinition number_consumer{
        "Number", "1.0.0", "unary", "print", 1, 1,
        {{"value", "core.number", "core.number"}},
        {{"output", "core.number", "core.number"}}};
    if (!nysor::validate_connection(producer, "event", event_consumer, "trigger").compatible) {
      throw std::runtime_error("matching specifications were rejected");
    }
    if (nysor::validate_connection(producer, "event", number_consumer, "value").compatible) {
      throw std::runtime_error("incompatible specifications were accepted");
    }
    std::cout << "Event -> Event: PASS\nEvent -> Number: FAIL as expected\nSpecification compatibility: PASS\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "FAILED: " << error.what() << '\n';
    return 1;
  }
}
