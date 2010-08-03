#include "Database.h"

using namespace Deska;
using namespace DB;

int main(int argc, char** argv) {

Database db;
db.startTransaction();
db.getBoxModelNames();

return 0;
}