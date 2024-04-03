#pragma once

extern void create_application(application&);

int main() {
    application app;

    create_application(app);

    if(!fabric::initialize(app)) {
        FBFATAL("Critical failure while initializing Fabric! Aborting.");

        goto termination;
    }

    if(!fabric::update(app)) {
        FBFATAL("Critical failure while updating Fabric! Aborting.");

        goto termination;
    }

termination:
    fabric::terminate(app);
    
    return 0;
}