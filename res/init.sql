create table nodes(
    id integer primary key autoincrement, 
    uuid text unique not null,
    name text not null
);

create table chat(
    id integer primary key autoincrement,
    uuid text unique not null, 
    timestamp datetime not null, 
    msg text not null, 
    foreign key(uuid) references nodes(uuid)
);
