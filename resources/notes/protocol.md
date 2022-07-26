# Computational Data Management System


## Overview - Architecture and Protocol

The "protocol" describes the general communication flow between various components in the
various systems we envision mohair being relevant for. It is difficult to discuss the
protocol without also talking about the organization of components in our system; so, we
also discuss the various architectures we consider and their relationships.


### Architecture

Based on research done for [MSG Express][slides-msgexpress], we can imagine a
Computational Data Management System to consist of three important layers: (1) storage
clients, (2) computational storage servers, and (3) computational storage devices.

At the lowest level of abstraction, computational storage devices (CSDs) refer to storage
devices (e.g. HDDs or SSDs) that have an attached accelerator or SoC which allows them to
do some processing without the use of a CPU complex or main memory.

Above the CSDs are computational storage servers. These differ from typical storage
servers only in their ability to propagate, or delegate, data processing to underlying
CSDs. This means that the difference is primarily in software and control logic, rather
than hardware. Although we explicitly refer to them as computational storage servers here,
throughout our writing and documentation in this repository we simply call them storage
servers.

Finally, at the highest level of abstraction are storage clients. These are clients that
are co-designed with the storage system so that they can directly communicate to storage
servers for efficiency, but they are physically on a compute server (either application
servers or edge servers). Storage clients act as the interface to the computational data
management system: they accept queries from applications, then manage access to data
partitioned across storage servers and merge result sets before returning them to an
application.


<!-- resources -->
[slides-msgexpress]: https://docs.google.com/presentation/d/1Nollf087CRhMmEAWcwfudIizIhF-ttPRGgaqmuXtSBQ/edit?usp=sharing
