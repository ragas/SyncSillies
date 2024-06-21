import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.pyplot import savefig

df = pd.read_csv('../performance.csv')

sizes = [64, 128, 256, 512]
parallelism_ticks = [1, 2, 4, 8, 16, 32, 64]
work_ticks = [1, 2, 4, 8, 16, 32, 64, 128, 256, 512]

dimens = ['parallelism', 'max_work_size']
count = 2
for dimen in dimens:
    count = count + 1
    fig = plt.figure(count, figsize=(8, 10))
    i = 0
    for size in sizes:
        i += 1
        df_sized = df[
            (df['size'] == size) &
            ((df['max_work_size'] == 32) if dimen == 'parallelism' else (df['parallelism'] == 8)) &
            (df['tag'] == 'work_size')]

        baseline = df[
            (df['size'] == size) &
            (df['tag'] == 'baseline')]['duration']

        p = plt.subplot(2, 2, i)
        p.set_yscale('log')
        p.set_ylabel('nanoseconds')
        p.set_xscale('log', base=2)
        p.set_xticks(parallelism_ticks if dimen == 'parallelism' else work_ticks)
        p.set_title("Matrix Size {}".format(size))
        df_sized[[dimen, 'duration']].plot(
            x=dimen, ax=p, style=['g+-'])
        p.grid()
        plt.axhline(y=baseline.iloc[0], color='r', linestyle='--')
        b = p.twinx()
        b.set_ylabel('steals')
        df_sized[[dimen, 'steal_count']].plot(x=dimen, ax=b, style=['b+-'])
        b.grid(visible=False)
        p.get_legend().remove()
        b.get_legend().remove()

    labels = ['duration', 'baseline', 'steal_count']
    fig.legend(labels, loc='lower right', ncol=len(labels))
    fig.tight_layout(pad=3.0)
    savefig('{}.svg'.format(dimen), dpi=300)
