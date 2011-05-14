#include <iostream>
#include <string>
#include "deska/db/Connection.h"
#include "CliInteraction.h"
#include "ParserSignals.h"
#include "UserInterface.h"
#include "Parser.h"

int main()
{
    Deska::Db::Connection conn;
    Deska::Cli::Parser parser(&conn);
    Deska::Cli::CliInteraction db(&conn);
    Deska::Cli::UserInterface ui(std::cout, std::cerr, std::cin, &db, &parser);
    Deska::Cli::SignalsHandler(&parser, &ui);
    ui.run();
    return 0;
}
