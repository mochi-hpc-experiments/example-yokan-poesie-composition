#include <stdio.h>
#include <stdlib.h>
#include <yokan/client.h>
#include <yokan/database.h>
#include <poesie/client.h>

static void write_with_yokan(margo_instance_id, hg_addr_t addr);
static void read_with_poesie(margo_instance_id, hg_addr_t addr);

int main(int argc, char** argv) {

    if(argc != 2) {
        fprintf(stderr, "Usage: %s <address>\n", argv[0]);
        exit(-1);
    }

    margo_instance_id mid = margo_init("na+sm", MARGO_CLIENT_MODE, 0, 0);

    hg_addr_t addr = HG_ADDR_NULL;
    margo_addr_lookup(mid, argv[1], &addr);

    write_with_yokan(mid, addr);
    read_with_poesie(mid, addr);

    margo_addr_free(mid, addr);
    margo_finalize(mid);
}

static void write_with_yokan(margo_instance_id mid, hg_addr_t addr) {
    yk_client_t client = YOKAN_CLIENT_NULL;
    yk_client_init(mid, &client);

    yk_database_handle_t db = YOKAN_DATABASE_HANDLE_NULL;
    yk_database_id_t db_id;
    yk_database_find_by_name(client, addr, 0, "my_kv_store", &db_id);

    yk_database_handle_create(client, addr, 0, db_id, &db);

    const char* key = "my_key";
    const char* val = "some_value";

    yk_put(db, YOKAN_MODE_DEFAULT,
           key, strlen(key),
           val, strlen(val));

    yk_database_handle_release(db);
    yk_client_finalize(client);
}

static const char* code = R"python(
value_len = database.length("my_key")
value_buf = bytearray(value_len)
database.get("my_key", value_buf)
__poesie_output__ = value_buf.decode("utf-8")
)python";

static void read_with_poesie(margo_instance_id mid, hg_addr_t addr) {
    poesie_client_t client = POESIE_CLIENT_NULL;
    poesie_client_init(mid, &client);

    poesie_provider_handle_t ph = POESIE_PROVIDER_HANDLE_NULL;
    poesie_provider_handle_create(client, addr, 0, &ph);

    poesie_vm_id_t vm;
    poesie_lang_t lang;
    poesie_get_vm_info(ph, "my_python_vm", &vm, &lang);

    char* output = NULL;
    poesie_execute(ph, vm, lang, code, &output);

    printf("%s\n", output);

    free(output);

    poesie_provider_handle_release(ph);
    poesie_client_finalize(client);
}
