#include "check.h"

#include <stdlib.h>

extern Suite* private_ACUtilsTest_ADynArray_getTestSuite(void);

int main(void)
{
    int numberFailed;
    SRunner *runner;

    runner = srunner_create(private_ACUtilsTest_ADynArray_getTestSuite());
    srunner_set_fork_status(runner, CK_NOFORK);
    srunner_run_all(runner, CK_NORMAL);
    numberFailed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (numberFailed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
