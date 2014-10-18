/** LMDB Matlab wrapper.
 */
#include <lmdb.h>
#include <memory>
#include <mexplus.h>

using namespace std;
using namespace mexplus;

#define ERROR(...) \
    mexErrMsgIdAndTxt("lmdb:error", __VA_ARGS__)
#define ASSERT(cond, ...) \
    if (!(cond)) mexErrMsgIdAndTxt("lmdb:error", __VA_ARGS__)

namespace {

// Transaction manager.
class Transaction {
public:
  // Create an empty transaction.
  Transaction() : txn_(NULL) {}
  // Shorthand for constructor-begin.
  Transaction(MDB_env* env, unsigned int flags) : txn_(NULL) {
    begin(env, flags);
  }
  virtual ~Transaction() { abort(); }
  // Begin the transaction.
  void begin(MDB_env* env, unsigned int flags) {
    ASSERT(env, "Null pointer exception.");
    abort();
    int status = mdb_txn_begin(env, NULL, flags, &txn_);
    ASSERT(status == MDB_SUCCESS, mdb_strerror(status));
  }
  // Commit the transaction.
  void commit() {
    if (txn_) {
      int status = mdb_txn_commit(txn_);
      ASSERT(status == MDB_SUCCESS, mdb_strerror(status));
    }
    txn_ = NULL;
  }
  // Abort the transaction.
  void abort() {
    if (txn_)
      mdb_txn_abort(txn_);
    txn_ = NULL;
  }
  // Get the raw transaction pointer.
  MDB_txn* get() { return txn_; }

private:
  // MDB_txn pointer.
  MDB_txn* txn_;
};

// Cursor container.
class Cursor {
public:
  Cursor() : cursor_(NULL) {}
  virtual ~Cursor() { close(); }
  // Open the cursor.
  void open(MDB_txn *txn, MDB_dbi dbi) {
    close();
    int status = mdb_cursor_open(txn, dbi, &cursor_);
    ASSERT(status == MDB_SUCCESS, mdb_strerror(status));
  }
  // Close the cursor.
  void close() {
    if (cursor_)
      mdb_cursor_close(cursor_);
    cursor_ = NULL;
  }
  // Get the raw cursor.
  MDB_cursor* get() { return cursor_; }

private:
  // MDB_cursor pointer.
  MDB_cursor* cursor_;
};

// Database manager.
class Database {
public:
  // Create an empty database environment.
  Database() : env_(NULL) {
    int status = mdb_env_create(&env_);
    ASSERT(status == MDB_SUCCESS, mdb_strerror(status));
  }
  virtual ~Database() { close(); }
  // Open an environment.
  void openEnv(const char* filename, unsigned int flags, mdb_mode_t mode) {
    ASSERT(env_, "MDB_env not created.");
    int status = mdb_env_open(env_, filename, flags, mode);
    ASSERT(status == MDB_SUCCESS, mdb_strerror(status));
  }
  // Open a table.
  void openDBI(MDB_txn* txn, const char* name, unsigned int flags) {
    ASSERT(env_, "MDB_env not opened.");
    int status = mdb_dbi_open(txn, name, flags, &dbi_);
    ASSERT(status == MDB_SUCCESS, mdb_strerror(status));
  }
  // Close both the table and the environment.
  void close() {
    if (env_) {
      mdb_dbi_close(env_, dbi_);
      mdb_env_close(env_);
    }
    env_ = NULL;
  }
  // Set the mapsize.
  void setMapsize(size_t mapsize) {
    int status = mdb_env_set_mapsize(env_, mapsize);
    ASSERT(status == MDB_SUCCESS, mdb_strerror(status));
  }
  // Get the raw MDB_env pointer.
  MDB_env* getEnv() { return env_; }
  // Get the raw MDB_dbi pointer.
  MDB_dbi getDBI() { return dbi_; }

private:
  // MDB_env pointer.
  MDB_env* env_;
  // MDB_dbi pointer.
  MDB_dbi dbi_;
};

// Record wrapper.
class Record {
public:
  // Create an empty record.
  Record() {
    mdb_val_.mv_size = 0;
    mdb_val_.mv_data = NULL;
  }
  // Create from string.
  Record(const string& data) {
    initialize(data);
  }
  virtual ~Record() {}
  // Initialize with string.
  void initialize(const string& data) {
    data_.assign(data.begin(), data.end());
    mdb_val_.mv_size = data_.size();
    mdb_val_.mv_data = const_cast<char*>(data_.c_str());
  }
  // Get MDB_val pointer.
  MDB_val* get() { return &mdb_val_; }
  // Beginning of iterator.
  const char* begin() const {
    return reinterpret_cast<const char*>(mdb_val_.mv_data);
  }
  // End of iterator.
  const char* end() const { return begin() + mdb_val_.mv_size; }

private:
  // Data buffer.
  string data_;
  // Dumb MDB_val.
  MDB_val mdb_val_;
};

} // namespace

namespace mexplus {

// Template specialization of mxArray* to Record.
template <>
void MxArray::to(const mxArray* array, Record* value) {
  ASSERT(value, "Null pointer exception.");
  value->initialize(MxArray(array).to<string>());
}

// Template specialization of Record to mxArray*.
template <>
mxArray* MxArray::from(const Record& value) {
  return MxArray(string(value.begin(), value.end())).release();
}

// Session instance storage.
template class Session<Database>;

} // namespace mexplus

namespace {

MEX_DEFINE(new) (int nlhs, mxArray* plhs[],
                 int nrhs, const mxArray* prhs[]) {
  InputArguments input(nrhs, prhs, 1, 20, "MODE", "FIXEDMAP", "NOSUBDIR",
      "NOSYNC", "RDONLY", "NOMETASYNC", "WRITEMAP", "MAPASYNC", "NOTLS",
      "NOLOCK", "NORDAHEAD", "NOMEMINIT", "REVERSEKEY", "DUPSORT",
      "INTEGERKEY", "DUPFIXED", "INTEGERDUP", "REVERSEDUP", "CREATE",
      "MAPSIZE");
  OutputArguments output(nlhs, plhs, 1);
  std::unique_ptr<Database> database(new Database);
  ASSERT(database.get() != NULL, "Null pointer exception.");
  database->setMapsize(input.get<size_t>("MAPSIZE", 10485760));
  string filename(input.get<string>(0));
  mdb_mode_t mode = input.get<int>("MODE", 0664);
  unsigned int flags =
      ((input.get<bool>("FIXEDMAP", false)) ? MDB_FIXEDMAP : 0) |
      ((input.get<bool>("NOSUBDIR", false)) ? MDB_NOSUBDIR : 0) |
      ((input.get<bool>("NOSYNC", false)) ? MDB_NOSYNC : 0) |
      ((input.get<bool>("RDONLY", false)) ? MDB_RDONLY : 0) |
      ((input.get<bool>("NOMETASYNC", false)) ? MDB_NOMETASYNC : 0) |
      ((input.get<bool>("WRITEMAP", false)) ? MDB_WRITEMAP : 0) |
      ((input.get<bool>("MAPASYNC", false)) ? MDB_MAPASYNC : 0) |
      ((input.get<bool>("NOTLS", false)) ? MDB_NOTLS : 0) |
      ((input.get<bool>("NOLOCK", false)) ? MDB_NOLOCK : 0) |
      ((input.get<bool>("NORDAHEAD", false)) ? MDB_NORDAHEAD : 0) |
      ((input.get<bool>("NOMEMINIT", false)) ? MDB_NOMEMINIT : 0);
  database->openEnv(filename.c_str(), flags, mode);
  flags = ((input.get<bool>("RDONLY", false)) ? MDB_RDONLY : 0);
  Transaction transaction(database->getEnv(), flags);
  flags =
      ((input.get<bool>("REVERSEKEY", false)) ? MDB_REVERSEKEY : 0) |
      ((input.get<bool>("DUPSORT", false)) ? MDB_DUPSORT : 0) |
      ((input.get<bool>("INTEGERKEY", false)) ? MDB_INTEGERKEY : 0) |
      ((input.get<bool>("DUPFIXED", false)) ? MDB_DUPFIXED : 0) |
      ((input.get<bool>("INTEGERDUP", false)) ? MDB_INTEGERDUP : 0) |
      ((input.get<bool>("REVERSEDUP", false)) ? MDB_REVERSEDUP : 0) |
      ((input.get<bool>("CREATE", flags == 0)) ? MDB_CREATE : 0);
  database->openDBI(transaction.get(), NULL, flags);
  transaction.commit();
  output.set(0, Session<Database>::create(database.release()));
}

MEX_DEFINE(delete) (int nlhs, mxArray* plhs[],
                    int nrhs, const mxArray* prhs[]) {
  InputArguments input(nrhs, prhs, 1);
  OutputArguments output(nlhs, plhs, 0);
  Session<Database>::destroy(input.get(0));
}

MEX_DEFINE(get) (int nlhs, mxArray* plhs[],
                 int nrhs, const mxArray* prhs[]) {
  InputArguments input(nrhs, prhs, 2);
  OutputArguments output(nlhs, plhs, 1);
  Database* database = Session<Database>::get(input.get(0));
  Record key = input.get<Record>(1);
  Record value;
  Transaction transaction(database->getEnv(), MDB_RDONLY);
  int status = mdb_get(transaction.get(),
                       database->getDBI(),
                       key.get(),
                       value.get());
  ASSERT(status == MDB_SUCCESS || status == MDB_NOTFOUND,
         mdb_strerror(status));
  transaction.commit();
  output.set(0, value);
}

MEX_DEFINE(put) (int nlhs, mxArray* plhs[],
                 int nrhs, const mxArray* prhs[]) {
  InputArguments input(nrhs, prhs, 3, 4, "NODUPDATA", "NOOVERWRITE", "RESERVE",
      "APPEND");
  OutputArguments output(nlhs, plhs, 0);
  Database* database = Session<Database>::get(input.get(0));
  unsigned int flags =
      ((input.get<bool>("NODUPDATA", false)) ? MDB_NODUPDATA : 0) |
      ((input.get<bool>("NOOVERWRITE", false)) ? MDB_NOOVERWRITE : 0) |
      ((input.get<bool>("RESERVE", false)) ? MDB_RESERVE : 0) |
      ((input.get<bool>("APPEND", false)) ? MDB_APPEND : 0);
  Record key = input.get<Record>(1);
  Record value = input.get<Record>(2);
  Transaction transaction(database->getEnv(), 0);
  int status = mdb_put(transaction.get(),
                       database->getDBI(),
                       key.get(),
                       value.get(),
                       flags);
  ASSERT(status == MDB_SUCCESS, mdb_strerror(status));
  transaction.commit();
}

MEX_DEFINE(remove) (int nlhs, mxArray* plhs[],
                    int nrhs, const mxArray* prhs[]) {
  InputArguments input(nrhs, prhs, 2);
  OutputArguments output(nlhs, plhs, 0);
  Database* database = Session<Database>::get(input.get(0));
  Record key = input.get<Record>(1);
  Transaction transaction(database->getEnv(), 0);
  int status = mdb_del(transaction.get(), database->getDBI(), key.get(), NULL);
  ASSERT(status == MDB_SUCCESS, mdb_strerror(status));
  transaction.commit();
}

MEX_DEFINE(each) (int nlhs, mxArray* plhs[],
                  int nrhs, const mxArray* prhs[]) {
  InputArguments input(nrhs, prhs, 2);
  OutputArguments output(nlhs, plhs, 0);
  Database* database = Session<Database>::get(input.get(0));
  Cursor cursor;
  Record key, value;
  Transaction transaction(database->getEnv(), MDB_RDONLY);
  cursor.open(transaction.get(), database->getDBI());
  int status = mdb_cursor_get(cursor.get(), key.get(), value.get(), MDB_NEXT);
  while (status == MDB_SUCCESS) {
    MxArray key_array(key);
    MxArray value_array(value);
    mxArray* prhs[] = {const_cast<mxArray*>(input.get(1)),
                       const_cast<mxArray*>(key_array.get()),
                       const_cast<mxArray*>(value_array.get())};
    ASSERT(mexCallMATLAB(0, NULL, 3, prhs, "feval") == 0, "Callback failure.");
    status = mdb_cursor_get(cursor.get(), key.get(), value.get(), MDB_NEXT);
  }
  ASSERT(status == MDB_SUCCESS || status == MDB_NOTFOUND,
         mdb_strerror(status));
  cursor.close();
  transaction.commit();
}

MEX_DEFINE(reduce) (int nlhs, mxArray* plhs[],
                    int nrhs, const mxArray* prhs[]) {
  InputArguments input(nrhs, prhs, 3);
  OutputArguments output(nlhs, plhs, 1);
  Database* database = Session<Database>::get(input.get(0));
  Cursor cursor;
  Record key, value;
  MxArray accumulation(input.get(2));
  Transaction transaction(database->getEnv(), MDB_RDONLY);
  cursor.open(transaction.get(), database->getDBI());
  int status = mdb_cursor_get(cursor.get(), key.get(), value.get(), MDB_NEXT);
  while (status == MDB_SUCCESS) {
    MxArray key_array(key);
    MxArray value_array(value);
    mxArray* lhs = NULL;
    mxArray* prhs[] = {const_cast<mxArray*>(input.get(1)),
                       const_cast<mxArray*>(key_array.get()),
                       const_cast<mxArray*>(value_array.get()),
                       const_cast<mxArray*>(accumulation.get())};
    ASSERT(mexCallMATLAB(1, &lhs, 4, prhs, "feval") == 0, "Callback failure.");
    accumulation.reset(lhs);
    status = mdb_cursor_get(cursor.get(), key.get(), value.get(), MDB_NEXT);
  }
  ASSERT(status == MDB_SUCCESS || status == MDB_NOTFOUND,
         mdb_strerror(status));
  cursor.close();
  transaction.commit();
  output.set(0, accumulation.release());
}

} // namespace

MEX_DISPATCH
