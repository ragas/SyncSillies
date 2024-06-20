import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.pyplot import savefig

df = pd.read_csv('performance.csv')

sizes = [64, 128, 256, 512]


fig = plt.figure(1, figsize=(10, 6))
plt.grid(visible=True, which='major')

i = 0
for size in sizes:
    i += 1
    df_sized = df[
      (df['size'] == size) &
      (df['max_work_size'] == 32) &
      (df['tag'] == 'work_size')]

    baseline = df[
      (df['size'] == size) &
      (df['tag'] == 'baseline')]['duration']

    p = plt.subplot(2, 2, i)

    #p.locator_params(nbins=12)
    p.set_title("Size {}".format(size))
    df_sized[['parallelism', 'duration']].plot(
        x='parallelism', ax=p, color=['green', 'blue'])
    plt.axhline(y=baseline.iloc[0], color='r', linestyle='-')
    b = p.twinx()
    df_sized[['parallelism', 'steal_count']].plot(x='parallelism', ax=b)
    p.get_legend().remove()
    b.get_legend().remove()

labels = ['duration', 'baseline', 'steal_count']
fig.legend(labels, loc='lower right', ncol=len(labels))
fig.tight_layout(pad=3.0)
savefig('parallelism.svg', transparent=True, dpi=300)

fig2 = plt.figure(2)

i = 0
for size in sizes:
    i += 1
    df_sized = df[
        (df['size'] == size) &
        (df['parallelism'] == 8) &
        (df['tag'] == 'work_size')]

    baseline = df[
        (df['size'] == size) &
        (df['tag'] == 'baseline')]['duration']

    p = plt.subplot(2, 2, i)
    p.set_title("Size {}".format(size))
    df_sized[['max_work_size', 'duration']].plot(
        x='max_work_size', ax=p, color=['green', 'blue'])
    plt.axhline(y=baseline.iloc[0], color='r', linestyle='-')
    b = p.twinx()
    df_sized[['max_work_size', 'steal_count']].plot(x='max_work_size', ax=b)
    p.get_legend().remove()
    b.get_legend().remove()


labels = ['duration', 'baseline', 'steal_count']
fig2.legend(labels, loc='lower right', ncol=len(labels))
fig2.tight_layout(pad=3.0)
savefig('work_break.svg', transparent=True, dpi=300)
