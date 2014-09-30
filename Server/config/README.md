# Configuration

Base configuration is performed in [the defaults configuration
file](defaults.json). Any data in the defaults file can be overriden
with an environment-specific configuration file following the naming
convention `config.<environment>.json`. The environment can be selected by
settting the `NODE_ENV` environment variable, and defaults to
`production`. See "Defaults and Environments" below for more details.

## Defaults and Environments

The server configurations implement a simple inheritance mechanism to
reduce redundancy in basic configuration. The server can select an
environment to run in by setting the `NODE_ENV` environment variable
before running the server:

    set NODE_ENV=development
    node server.js

Setting the `NODE_ENV` selects the configuration file to load by looking
for a file following the convention `config.<environment>.json`. If no
`NODE_ENV` is specified, it will default to `production`.

Each environment's configuration is inherited from two kinds of
defaults: global defaults and inline defaults. Both kinds of defaults
are merged into the environment-specific configuration, giving
precedence in the following order:

1. the non-default value (highest precedence)
2. inline defaults
3. global defaults (lowest precedence)

Global defaults are specified in the file `defaults.json`. Inline
defaults are specified by creating a configuration value with the same
name as the target key, prefixed with a dollar sign. Given the following
configuration files for the `production` environment:

    # defaults.json
    {
      "$array": { "a": 1 },
      "array": [],
      "deepMerge": { "x": { "y": 100 } }
    }

    # config.production.json
    {
      "$array" : { "b": 2 },
      "array": [ { "c": 3 }, { "a": "I WIN!", "c": 4 } ],
      "deepMerge": { "x": { "z": 200 }, "zz": 300 }
    }

The resulting configuration would be:

    {
      "array": [ { "a": 1, "b": 2, "c": 3 }, { "a": "I WIN!", "b": 2, "c": 4 } ]
      "deepMerge": { "x": { "y": 100, "z": 200 }, "zz": 300 }
    }

As you can see in the example, the inline defaults are sensitive to
arrays. If the defaulted setting is an array, the defaults will be
merged into every element, otherwise they are merged directly into the
object.

Generally you don't need to worry about this. Just be aware that you can
set a configuration variable everywhere by putting it intto the defaults
file, or a key prefixed with a dollar sign. The current configurations
already define these keys in the places they're most likely to be used,
so you can just build off of that.

See [config.js](../config.js) for the implementation of this logic.

## Configuration Dictionary

| Path                                     | Description
|------------------------------------------|------------
|`osc_server_host`                         |address OSC should bind to to receive OSC messages
|`osc_server_port`                         |port OSC should bind to to receive OSC messages
|`web_port`                                |port Socket.io binds to
|`$clients`                                |merged into every client's config. See `clients[].*` for valid values, and Defaults (above) for an explanation of how defaults work.
|`clients`                                 |an array of client configuration details
|`clients[].host`                          |the OSC host the client is listening on
|`clients[].port`                          |the OSC port the client is listening on