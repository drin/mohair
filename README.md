# Overview

"Mohair" is a project to prototype the use of [substrait][web-substrait] and
[arrow][web-arrow] to handle pushdown of partial queries to remote storage. Initially,
this project will largely inspired by [Skytether][repo-skytether]--an extension of
[SkyhookDM][repo-skyhookdm] for single-cell gene expression analysis and computational
storage. However, work on skytether narrowly started before [Arrow Flight][docs-flight],
[Acero][docs-acero], and substrait were started. So, a lot of skytether is low-level and
specific, where mohair will try to be higher-level and more generic.

# Goals

The overall goal is to be able to delegate part of a query plan to remote storage, execute
the remaining query plan on the intermediate results (from remote storage), and then
return the final results to the client. It is expected that this will require some amount
of:
* splitting a query plan
* implementing a flight service
* communicating with a remote flight service
* communicating with flight services using substrait

## Milestones

For an informal tracking of progress, we list some milestones here (which will be updated
as appropriate).

1. Execute a single query without splitting
   1. Submit query as a substrait plan
   2. Submit query to a flight service
   3. Implement flight service as a very simple data server (probably using a file system
      for now)
2. Execute a single query with a simple split
   1. Submit query as a substrait plan
   2. Split plan into 2 pieces
   3. Execute the 2nd piece on the intermediate results of the 1st piece.


<!-- resources -->
[web-substrait]:  https://substrait.io/
[web-arrow]:      https://arrow.apache.org/

[docs-flight]:    https://arrow.apache.org/docs/format/Flight.html
[docs-acero]:     https://arrow.apache.org/docs/cpp/streaming_execution.html

[repo-skytether]: https://gitlab.com/skyhookdm/skytether-singlecell
[repo-skyhookdm]: https://github.com/uccross/skyhookdm-ceph-cls
