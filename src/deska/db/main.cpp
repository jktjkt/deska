#include "deska/db/Connection.h"

int main()
{
    Deska::Db::Connection conn;
    //conn.abortCurrentChangeset();
    conn.kindRelations("a");
    return 0;
}
