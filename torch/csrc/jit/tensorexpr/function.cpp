#include <torch/csrc/jit/tensorexpr/function.h>

#include <c10/util/Logging.h>
#include <torch/csrc/jit/tensorexpr/tensor.h>

namespace torch {
namespace jit {
namespace tensorexpr {

namespace {

static void unpack_dim_args(
    const std::vector<DimArg>& dim_args,
    std::vector<const Expr*>* dims,
    std::vector<const Var*>* vars) {
  dims->clear();
  vars->clear();
  for (const DimArg& dim_arg : dim_args) {
    dims->push_back(dim_arg.dim().node());
    vars->push_back(new Var(dim_arg.name_hint(), kInt));
  }
}

} // namespace

Tensor* Compute(
    const std::string& func_name,
    const std::vector<DimArg>& dim_args,
    const std::function<ExprHandle(const std::vector<VarHandle>&)>& body_func) {
  std::vector<const Expr*> dims;
  std::vector<const Var*> args;
  unpack_dim_args(dim_args, &dims, &args);
  const Expr* body = body_func(VarVectorToVarHandleVector(args)).node();
  Function* func = new Function(func_name, dims, args, body);
  const Buf* buf = func->func_var(0);
  return new Tensor(buf, func, 0);
}

Tensor* Compute(
    const std::string& func_name,
    const std::vector<DimArg>& dim_args,
    const std::function<ExprHandle(const VarHandle&)>& body_func) {
  if (dim_args.size() != 1) {
    throw malformed_input();
  }

  std::vector<const Expr*> dims;
  std::vector<const Var*> args;
  unpack_dim_args(dim_args, &dims, &args);
  const Expr* body = body_func(VarHandle(args[0])).node();
  Function* func = new Function(func_name, dims, args, body);
  const Buf* buf = func->func_var(0);
  return new Tensor(buf, func, 0);
}

Tensor* Compute(
    const std::string& func_name,
    const std::vector<DimArg>& dim_args,
    const std::function<ExprHandle(const VarHandle&, const VarHandle&)>&
        body_func) {
  if (dim_args.size() != 2) {
    throw malformed_input();
  }
  std::vector<const Expr*> dims;
  std::vector<const Var*> args;
  unpack_dim_args(dim_args, &dims, &args);
  const Expr* body = body_func(VarHandle(args[0]), VarHandle(args[1])).node();
  Function* func = new Function(func_name, dims, args, body);
  const Buf* buf = func->func_var(0);
  return new Tensor(buf, func, 0);
}

Tensor* Compute(
    const std::string& func_name,
    const std::vector<DimArg>& dim_args,
    const std::function<
        ExprHandle(const VarHandle&, const VarHandle&, const VarHandle&)>&
        body_func) {
  if (dim_args.size() != 3) {
    throw malformed_input();
  }
  std::vector<const Expr*> dims;
  std::vector<const Var*> args;
  unpack_dim_args(dim_args, &dims, &args);
  const Expr* body =
      body_func(VarHandle(args[0]), VarHandle(args[1]), VarHandle(args[2]))
          .node();
  Function* func = new Function(func_name, dims, args, body);
  const Buf* buf = func->func_var(0);
  return new Tensor(buf, func, 0);
}

Tensor* Compute(
    const std::string& func_name,
    const std::vector<DimArg>& dim_args,
    const std::function<ExprHandle(
        const VarHandle&,
        const VarHandle&,
        const VarHandle&,
        const VarHandle&)>& body_func) {
  if (dim_args.size() != 4) {
    throw malformed_input();
  }
  std::vector<const Expr*> dims;
  std::vector<const Var*> args_nodes;
  unpack_dim_args(dim_args, &dims, &args_nodes);
  auto args = VarVectorToVarHandleVector(args_nodes);
  const Expr* body = body_func(args[0], args[1], args[2], args[3]).node();
  Function* func = new Function(func_name, dims, args_nodes, body);
  const Buf* buf = func->func_var(0);
  return new Tensor(buf, func, 0);
}

Stmt* Function::ElementStmt(size_t index) {
  const Buf* buf = func_var(index);
  std::vector<const Expr*> indices;
  for (size_t i = 0; i < buf->ndim(); i++) {
    indices.push_back(this->args_[i]);
  }

  const Expr* mask = new IntImm(1);

  Stmt* update_stmt = new Store(buf, indices, body(index), mask);
  return update_stmt;
}

} // namespace tensorexpr
} // namespace jit
} // namespace torch
