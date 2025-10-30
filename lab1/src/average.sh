!/bin/bash
sum=0
count=0

for num in "$@"; do
    sum=$((sum + num))
    count=$((count + 1))
done

average=$((sum / count))

echo "Количество чисел: $count"
echo "Среднее арифметическое: $averge"
