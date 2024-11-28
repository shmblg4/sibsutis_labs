#!/bin/bash

NAME="my-nginx"
docker start $NAME

function check_isolation {
    echo "----------- Проверка уровней изоляции -----------"
    
    # PID изоляция
    echo "PID в контейнере:"
    docker top $NAME | grep "nginx"

    # IPC изоляция
    echo -e "\nIPC: Попробуем создать shared memory в контейнере:"
    docker exec $NAME bash -c "ipcs -m"

    # Network изоляция
    echo -e "\nNetwork: Получаем IP контейнера:"
    docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' $NAME

    # User изоляция
    echo -e "\nUser: Проверка пользователей внутри контейнера:"
    docker exec $NAME bash -c "cat /etc/passwd"

    # Mount изоляция
    echo -e "\nMount: Проверка монтированного пространства:"
    docker exec $NAME bash -c "df -h"

    # UTS изоляция
    echo -e "\nUTS: Проверка имени хоста:"
    docker exec $NAME bash -c "hostname"
}

check_isolation

docker stop $NAME