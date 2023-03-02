# This is a demo for wrapped bindings.
# It defines a simple HN api binding that wraps the internet service.

using Workerd = import "/workerd/workerd.capnp";

const wrappedBindingExample :Workerd.Config = (
  services = [ (name = "main", worker = .main) ],
  sockets = [ (name = "http", address = "*:8080", http = (), service = "main") ],
);

const main :Workerd.Worker = (
  modules = [
    (name = "worker", esModule = embed "worker.js"),
    (name = "hnApi", esModule = embed "hnApi.js")
  ],
  compatibilityDate = "2022-09-16",
  bindings = [
    (
      # This binding will be accessible as `env.hn` the usual way.
      name = "hn",
      wrapped = (
        # `wrapBindings` function from hnApi module will be called to create the final binding.
        wrapWith = "hnApi",
        # Inner bindings are passed as `env` to the `wrapBindings` function.
        # In this cases it will be accessible as `env.internet`
        innerBindings = [ (name = "internet", service = "internet") ],
      )
    )
  ],
);
