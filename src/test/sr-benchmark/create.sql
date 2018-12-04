-- Table for storing basic rendering data.
CREATE TABLE rendering (
    n_proc INTEGER,          -- number of processors used during render.
    icet INTEGER,            -- IceTNMgr (1) or NMgr (0)?
    transparency INTEGER,    -- multipass rendering, thus transparent geometry
    n_gpus INTEGER,          -- number of GPUs used during rendering.
    n_cells INTEGER,         -- number of cells rendered
    n_pixels INTEGER,        -- number of pixels rendered
    r_time REAL              -- overall render time
);
