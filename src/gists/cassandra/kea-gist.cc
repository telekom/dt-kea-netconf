#include <config.h>

#include <cassandra.h>

int main() {
    CassCluster* cluster = cass_cluster_new();
    cass_cluster_free(cluster);

    return 0;
}
