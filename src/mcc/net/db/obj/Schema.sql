CREATE TABLE property
(
    name    text    PRIMARY KEY NOT NULL,
    value   text                NOT NULL
);

CREATE TABLE protocol
(
    id          integer PRIMARY KEY NOT NULL,
    name        text    UNIQUE      NOT NULL,
    info        text                NOT NULL,
    shareable   boolean             NOT NULL,
    logging     boolean             NOT NULL,
    param_info  text                NOT NULL,
    timeout     integer             NOT NULL,
    pixmap      blob
);

CREATE TABLE firmware
(
    id          integer PRIMARY KEY NOT NULL,
    name        text    UNIQUE      NOT NULL,
    info        text                NOT NULL,
    protocol_id integer             NOT NULL,
    binary      blob                NOT NULL,

    UNIQUE(protocol_id, info),

    FOREIGN KEY(protocol_id) REFERENCES protocol(id)
);

CREATE TABLE device_ui
(
    id          integer PRIMARY KEY NOT NULL,
    name        text    UNIQUE      NOT NULL,
    info        text                NOT NULL,
    protocol_id integer             NOT NULL,
    binary      blob                NOT NULL,

    UNIQUE(protocol_id, info),

    FOREIGN KEY(protocol_id) REFERENCES protocol(id)
);

CREATE TABLE device
(
    id              integer PRIMARY KEY NOT NULL,
    name            text    UNIQUE      NOT NULL,
    info            text    UNIQUE      NOT NULL,
    protocol_id     integer             NOT NULL,
    protocol_value  integer             NOT NULL,
    settings        text,
    firmware_id     integer,
    device_ui_id    integer,
    device_pixmap   blob,
    reg_first       boolean DEFAULT 0,
    show_on_map     boolean DEFAULT 1,
    log             boolean DEFAULT 0,

    UNIQUE(protocol_id, protocol_value),

    FOREIGN KEY(firmware_id)        REFERENCES firmware(id),
    FOREIGN KEY(protocol_id)        REFERENCES protocol(id),
    FOREIGN KEY(device_ui_id)       REFERENCES device_ui(id)
);

CREATE TABLE radar
(
    id              integer PRIMARY KEY NOT NULL,
    name            text    UNIQUE      NOT NULL,
    info            text    UNIQUE      NOT NULL,
    settings        text                NOT NULL
);

CREATE TABLE channel
(
    id              integer PRIMARY KEY NOT NULL,
    name            text    UNIQUE      NOT NULL,
    info            text    UNIQUE      NOT NULL,
    log             boolean             DEFAULT 0,
    isDynTimeout    boolean             DEFAULT 0,
    isReadOnly      boolean             DEFAULT 0,
    timeout         integer             NOT NULL,
    settings        text                NOT NULL,
    protocol_id     integer             NOT NULL,
    radar_id        integer                     ,
    reconnectTimeout integer                    ,

    FOREIGN KEY(protocol_id)    REFERENCES protocol(id),
    FOREIGN KEY(radar_id)       REFERENCES radar(id)
);

CREATE TABLE device_channel
(
    id                  integer PRIMARY KEY NOT NULL,
    device_id           integer             NOT NULL,
    channel_id          integer             NOT NULL,

    UNIQUE(channel_id, device_id),

    FOREIGN KEY(device_id)          REFERENCES device(id),
    FOREIGN KEY(channel_id)         REFERENCES channel(id)
);

CREATE TABLE tm_session
(
    id              integer PRIMARY KEY NOT NULL,
    name            text    UNIQUE      NOT NULL,
    info            text                NOT NULL,
    folder          text    UNIQUE      NOT NULL,
    started         text                NOT NULL,
    finished        text
)