# Computational Data Management System


## Overview - Architecture and Protocol

The "protocol" describes the general communication flow between various components in the
various systems we envision mohair being relevant for. It is difficult to discuss the
protocol without also talking about the organization of components in our system; so, we
also discuss the various architectures we consider and their relationships.

### Terminology

**Storage Components:**
* **Storage Device** -- A physical storage device such as a HDD or SSD or NVRam.
* **Storage Server** -- A physical or virtual machine that handles data storage and access
  and is **not** part of a compute cluster (may be part of a storage cluster, or a
  stand-alone server).
* **Storage Service** -- A process on a storage server that handles data storage and
  access. May be one-to-one with a storage server, or may represent a logical data shard
  if a storage server has many storage services.

**Computational Storage Components:**
* **Computational Storage Device** -- A storage device that has an attached CPU, SoC, or
  some other processor. Essentially, a storage device that can execute some compute
  independently of a client processor (storage server or storage client).
* **Computational Storage Service** -- A storage service that is capable of delegating
  work to a computational storage device and completing any remaining data processing
  tasks before returning data to the client.

**Clients:**
* **Client Process** -- A process that runs on a client server or compute server.
* **Storage Client** -- The portion of a client process that communicates with a storage
  service, computational storage service, or computational storage device. This may be
  something like a framework (skyhook or skytether) or a protocol (rados or gRPC).
* **Application Client** -- The portion of a client process where storage requests
  originate from.

**Processing:**
* **Computational Pushdown** -- Delegation of part, or all, of a data processing request
  to a separate processor.


### Architecture

Based on research done for [MSG Express][slides-msgexpress], we can imagine a
Computational Data Management System to consist of three important layers: (1) storage
clients, (2) computational storage services, and (3) computational storage devices.

At the lowest level of abstraction, computational storage devices (CSDs) refer to storage
devices (e.g. HDDs or SSDs) that have an attached accelerator or SoC which allows them to
do some processing without the use of the storage client's (or storage server's) CPU
complex or main memory.

Above the CSDs are computational storage services (CSSs). These differ from typical
storage services only in their ability to propagate, or delegate, data processing to
underlying CSDs. Note that CSSs and simple storage services can both run on "storage
servers." This means that the difference is primarily in software and control logic,
rather than hardware, and so we use "service" to refer to the software processes and
"server" to refer to the machine (physical or virtual). 

At the highest level of abstraction, storage clients are clients that send requests
directly to a storage service (CSS or simple). When we discuss storage clients that
communicate with a CSS, we assume that they are co-designed with the computational storage
system so that they can participate in end-to-end decisions; but, we also assume they are
physically resident on a compute server (either application servers or edge servers).
Storage clients act as the interface to the computational data management system: they
accept queries from applications, then manage access to data partitioned across storage
servers and merge result sets before returning them to an application.


#### Storage Systems

**Filesystem.** The simplest storage system is a storage server with an attached storage
device. In this architecture, a storage service (running on the storage server) can
receive data access requests from a client and serve those requests via an underlying
filesystem or file-like interface. The storage service can provide features such as data
filtering or projection before returning the data to the client. However, it is assumed
that the underlying filesystem is unable to receive complex requests (such as SQL queries
or a substrait query plan). This means that the storage service has no capability for
computational pushdowns: the underlying filesystem can only execute simple data accesses,
thus the storage server is where all data processing is executed before data is returned
to the client.

**Key Value Storage System.** This type of storage system associates a key name with a key
value. In this architecture, a storage service manages data for a subset of keys and can
serve data access requests for those keys. The underlying storage supports a simple data
access interface, treating key values as a single entity that may be retrieved
individually or as a group. This means that the interface cannot express computational
pushdowns.

**Object Storage System.** This type of storage system is similar to a key value storage
system, but associates object names with object blobs. Objects are like keys, but they
have a flexible interface. In this architecture, a storage service manages data for a
subset of objects and can serve data access requests for those objects. The underlying
storage is de-coupled, and so can be served by a filesystem, key-value storage, or other
storage backend. While the underlying storage backend may be able to receive complex
requests, it is assumed to not have an independent processor. This means that the storage
service cannot typically support computational pushdowns, but can be extended to support
them. Further, storage objects are identified by a single name, and any accesses at a
smaller granularity differ between storage systems.

**Computational Object Storage System.** This is an object storage system that consists of
a mix of computational storage services and standard storage services. In this
architecture, a standard storage device is associated with a standard storage service and
a computational storage device is associated with a computational storage service.
A standard storage services cannot support computational pushdown, but a computational
storage service does support computational pushdown by propagating data processing tasks
to its computational storage device.


#### Remote Computational Storage System

A computational storage system is the next level of complexity beyond a remote storage
system. The "computational" adjective denotes the capability for pushdowns to the
underlying storage. In other words, a computational storage service (running on the
storage server) communicates with an underlying storage service for data storage. It is
assumed that the underlying storage service is running on some sort of computational
storage device (CSD). The CSD may provide a low-level interface (e.g. smart SSDs) or a
high-level interface (e.g. appliances, kinetic drives).

While a computational storage service may simply propagate data access requests to the
underlying CSD, there is an opportunity for the storage service to manage or coordinate
the execution of a data access request with the CSD such that the overall performance when
serving the request is improved. In the simplest case, this means that a computational
storage service should be able to forward a data access request as-is to a CSD. In the
most complex case, this means that a computational storage service should be able to: (1)
alter portions of the data request into a different execution order, (2) send a portion of
the modified data access request to a CSD, and (3) interleave data access requests to a
CSD with processing of results from the CSD.

Finally, a CSD should be able to simplify execution of a data access when under load, and
a computational storage service should be able to adapt to the results returned from the
CSD to satisfy remaining portions of the data access request. In simpler terms, a CSD
should only run extra computations (e.g. filtering or projection) when the overhead is
within acceptable bounds relative to the required computations (e.g. read or consistent
writes). If the CSD does not run extra computations, then the computational storage
service should attempt to run the extra computations within similar constraints. In the
case when both the CSD and the computational storage service are under heavy load, it
should be possible to leave extra computations to the client (which is assumed to have a
more powerful compute complex).


### Mapping Storage Types to Flight Types

Apache Arrow defines an RPC protocol leveraging gRPC and the protobuf library. The
protocol and its implementation are collectively called Arrow Flight and is used when
communicating Arrow data for memory efficiency when sending Arrow data from a CPU to a NIC
(network interface card). Here, we discuss the mappings between a storage system's data
model and Arrow Flight's data model.

#### Control Flow Types

*FlightDescriptor.* A FlightDescriptor contains either string data or binary data. There
is a field, Type, which can be used to determine which type of data the FlightDescriptor
contains. The names of string and binary fields, respectively "path" and "cmd", suggests
that the FlightDescriptor represents some file name or application-specific identifier.

We imagine the FlightDescriptor to represent identifying information for a "dataset" (in
the general sense of the word). For a filesystem, this would likely be a file or directory
path. For an object storage system, this would likely be an object name or key. For a
key value storage system, this would likely be a key name. For a computational storage
system, we would like this to be either a simple name (e.g. an object name or key name for
Ceph) or a query (SQL or a substrait plan).

When delegating a data access request to an underlying CSD, we imagine a CSS to be able to
specify either a physical storage object (a kinetic key) or a computational namespace (a
skytether partition name).

*FlightEndpoint.*

*FlightInfo.*

#### Data Flow Actions

*ListFlights.*

*GetFlightInfo.*

*Get.*

*Put.*

*Exchange.*


<!-- resources -->
[slides-msgexpress]: https://docs.google.com/presentation/d/1Nollf087CRhMmEAWcwfudIizIhF-ttPRGgaqmuXtSBQ/edit?usp=sharing
