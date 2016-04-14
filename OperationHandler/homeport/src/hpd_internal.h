//
// Created by panic on 3/18/16.
//

#ifndef HOMEPORT_HPD_INTERNAL_H
#define HOMEPORT_HPD_INTERNAL_H

struct HomePort
{
    Configuration *configuration;
    struct ev_loop *loop;
};

#endif //HOMEPORT_HPD_INTERNAL_H
