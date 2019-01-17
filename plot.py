import matplotlib.pyplot as plt

# Random test data
all_data = [[],[]]

f = open("output")
data = []
for line in f:
    data.append(int(line))

f.close()

count = 0
while count < len(data):
    temp = [[],[]]
    for i in range(10):
        base = data[count]
        count += 1
        lock = data[count]
        count += 1
        tm = data[count]
        count += 1
        temp[0].append(float(base) / lock)
        temp[1].append(float(base) / tm)
    all_data[0].append(temp[0])
    all_data[1].append(temp[1])

print len(all_data)

fig, axes = plt.subplots(nrows=1, ncols=2, figsize=(9, 4))

# rectangular box plot
bplot1 = axes[0].boxplot(all_data[0],
        vert=True,   # vertical box aligmnent
        patch_artist=True)   # fill with color

# rectangular box plot
bplot2 = axes[1].boxplot(all_data[1],
        vert=True,   # vertical box aligmnent
        patch_artist=True)   # fill with color

# adding horizontal grid lines
axes[0].yaxis.grid(True)
axes[0].set_xticks([y+1 for y in range(len(all_data[0]))], )
axes[0].set_xlabel('threshold, probe_size')
axes[0].set_ylabel('speed')
axes[0].set_title('Lock Tests on XPS15 (8 threads, 16000 tasks, 50 initial capacity)')

# adding horizontal grid lines
axes[1].yaxis.grid(True)
axes[1].set_xticks([y+1 for y in range(len(all_data[1]))], )
axes[1].set_xlabel('threshold, probe_size')
axes[1].set_ylabel('speed')
axes[1].set_title('HTM Tests on XPS15 (8 threads, 16000 tasks, 50 initial capacity)')
    
# add x-tick labels
plt.setp(axes, xticks=[y+1 for y in range(len(all_data[0]))],
        xticklabels=['2, 4', '6, 8', '14, 16', '30, 32', '62, 64'])
plt.show()
