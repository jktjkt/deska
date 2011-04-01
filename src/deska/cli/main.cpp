#include <iostream>
#include <string>
#include "deska/db/Connection.h"
#include "CliInteraction.h"
#include "Parser.h"

int main()
{
    Deska::Db::Connection conn;
    Deska::Cli::Parser parser(&conn);
    Deska::Cli::CliInteraction cli(&conn, &parser);
    cli.run();
    return 0;
}
