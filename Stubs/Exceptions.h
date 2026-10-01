// STUB - Vimukthi owns the real version. Replace this file when his is ready.
#pragma once
#include <stdexcept>
#include <string>

struct InvalidInputException : std::runtime_error { using std::runtime_error::runtime_error; };
struct DuplicateIDException  : std::runtime_error { using std::runtime_error::runtime_error; };
struct ItemNotFoundException : std::runtime_error { using std::runtime_error::runtime_error; };
struct OutOfStockException   : std::runtime_error { using std::runtime_error::runtime_error; };
struct FileException         : std::runtime_error { using std::runtime_error::runtime_error; };
