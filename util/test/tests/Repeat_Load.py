import rdtest
import os
import psutil
import renderdoc as rd


class Repeat_Load(rdtest.TestCase):
    slow_test = True

    def repeat_load(self, path):
        memory_peak = memory_baseline = 0

        for i in range(20):
            rdtest.log.print("Loading for iteration {}".format(i))

            try:
                controller = rdtest.open_capture(path)
            except RuntimeError as err:
                rdtest.log.print("Skipping. Can't open {}: {}".format(path, err))
                return

            # Do nothing, just ensure it's loaded
            memory_usage: int = psutil.Process(os.getpid()).memory_info().rss

            # We measure the baseline memory usage during the second peak to avoid any persistent caches etc that might
            # not be full
            if i == 1:
                memory_baseline = memory_usage
            memory_peak = max(memory_peak, memory_usage)

            controller.Shutdown()

            pct_over = 'N/A'

            if memory_baseline > 0:
                pct_over = '{:.2f}%'.format((memory_usage / memory_baseline)*100)

            rdtest.log.success("Succeeded iteration {}, memory usage was {} ({} of baseline)"
                               .format(i, memory_usage, pct_over))

        pct_over = '{:.2f}%'.format((memory_peak / memory_baseline)*100)
        msg = 'peak memory usage was {}, {} compared to baseline {}'.format(memory_peak, pct_over, memory_baseline)

        if memory_baseline * 1.25 < memory_peak:
            raise rdtest.TestFailureException(msg)
        else:
            rdtest.log.success(msg)

    def run(self):
        dir_path = self.get_ref_path('', extra=True)

        for file in os.scandir(dir_path):
            rdtest.log.print('Repeat loading {}'.format(file.name))

            self.repeat_load(file.path)

            rdtest.log.success("Successfully repeat loaded {}".format(file.name))

        rdtest.log.success("Repeat loaded all files")
