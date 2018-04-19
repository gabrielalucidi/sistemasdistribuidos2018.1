<?php
$totalTime = 0;
for ($i=0; $i < 10; $i++) {
    $output = shell_exec('./producerConsumer.out 1 1');
    echo $output;
    $totalTime += $output;
}
$media = "Media 1-1 is:" . $totalTime/10;
echo $media;

$totalTime = 0;
for ($i=0; $i < 10; $i++) {
    $output = shell_exec('./producerConsumer.out 1 2');
    echo $output;
    $totalTime += $output;
}
$media = "Media 1-2 is:" . $totalTime/10;
echo $media;

$totalTime = 0;
for ($i=0; $i < 10; $i++) {
    $output = shell_exec('./producerConsumer.out 1 4');
    echo $output;
    $totalTime += $output;
}
$media = "Media 1-4 is:" . $totalTime/10;
echo $media;

$totalTime = 0;
for ($i=0; $i < 10; $i++) {
    $output = shell_exec('./producerConsumer.out 1 8');
    echo $output;
    $totalTime += $output;
}
$mdia = "Media 1-8 is:" . $totalTime/10;
echo $media;

$totalTime = 0;
for ($i=0; $i < 10; $i++) {
    $output = shell_exec('./producerConsumer.out 1 16');
    echo $output;
    $totalTime += $output;
}
$media = "Media 1-16 is:" . $totalTime/10;
echo $media;

$totalTime = 0;
for ($i=0; $i < 10; $i++) {
    $output = shell_exec('./producerConsumer.out 2 1');
    echo $output;
    $totalTime += $output;
}
$media = "Media 2-1 is:" . $totalTime/10;
echo $media;

$totalTime = 0;
for ($i=0; $i < 10; $i++) {
    $output = shell_exec('./producerConsumer.out 4 1');
    echo $output;
    $totalTime += $output;
}
$media = "Media 4-1 is:" . $totalTime/10;
echo $media;

$totalTime = 0;
for ($i=0; $i < 10; $i++) {
    $output = shell_exec('./producerConsumer.out 8 1');
    echo $output;
    $totalTime += $output;
}
$mdia = "Media 8-1 is:" . $totalTime/10;
echo $media;

$totalTime = 0;
for ($i=0; $i < 10; $i++) {
    $output = shell_exec('./producerConsumer.out 16 1');
    echo $output;
    $totalTime += $output;
}
$media = "Media 16-1 is:" . $totalTime/10;
echo $media;

