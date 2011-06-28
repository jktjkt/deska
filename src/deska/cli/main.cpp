#include <iostream>
#include <string>
#include "deska/db/Connection.h"
#include "DbInteraction.h"
#include "ParserSignals.h"
#include "UserInterface.h"
#include "UserInterfaceIO.h"
#include "Parser.h"

int main()
{
    Deska::Db::Connection conn;
    Deska::Cli::Parser parser(&conn);
    Deska::Cli::DbInteraction db(&conn);
    Deska::Cli::UserInterfaceIO io(&parser);
    Deska::Cli::UserInterface ui(&db, &parser, &io);
    Deska::Cli::SignalsHandler(&parser, &ui);
    ui.run();
    return 0;
}
