MathWebSearch Analytics
=======================

About
-----
This provides an interface for analyzing all expressions contained
in a MWS index.

Workflow
--------

Create a C++ source following this format and save it in
the [analytics/](.) directory. To access documentation on the API
functions, see [analytics.hpp](/src/mws/analytics/analytics.hpp).

``` cpp
#include "mws/analytics/analytics.hpp"

namespace mws {
namespace analytics {

AnalyticsStatus analyze_begin(const index_handle_t* index,
                              const inode_t* root) {
    // ...
    return ANALYTICS_OK;
}

AnalyticsStatus analyze_expression(const types::CmmlToken* cmmlToken,
                                   uint32_t num_hits) {
    // ...
    return ANALYTICS_OK;
}

void analyze_end() {
    // ...
}

}  // namespace analytics
}  // namespace mws
```

When building MWS, sources in `analytics/` will automatically generate
executable targets. As such, `example.cpp` will generate
`bin/analytics/analyze_example`. This can be run as:

    bin/analytics/analyze_example -I /path/to/saved/index/

To generate an index, run

    bin/mws-index -I /path/to/harvests -o /path/to/save/index

