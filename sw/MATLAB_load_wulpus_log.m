%% WULPUS Data Loading Script

%% Configuration
% Paths to measurement ZIP files
% paths needs to be a list of all zips, that should get concatinated.
% ´get_all_zips_from_folder´ can help

% paths = {'.\\wulpus\\measurements\\wulpus-2025-09-05_11-01-36.zip'};
paths = get_all_zips_from_folder('.\logs');
SAMPLE_CROP = 99999;  % Set to large number (e.g., 99999) for no crop

%% Load Data
[wulpus_data, config] = zip_to_dataframe(paths{1});

if length(paths) > 1
    fprintf('Multiple (%d) paths detected, concatenating DataFrames\n', length(paths));
    for i = 2:length(paths)
        [df_new, config_new] = zip_to_dataframe(paths{i});
        wulpus_data = concat_dataframes(wulpus_data, df_new);
        config = config_new;
        clear df_new config_new;
    end
    clear i;
end

fprintf('Loaded %d measurements\n', height(wulpus_data));
disp(wulpus_data(1:min(5, height(wulpus_data)), :));


%% Helper Functions

function paths = get_all_zips_from_folder(folder)
% Get all ZIP files from folder
files = dir(fullfile(folder, '*.zip'));
paths = arrayfun(@(f) fullfile(f.folder, f.name), files, 'UniformOutput', false);
end

function [df, config] = zip_to_dataframe(zip_path)
% Load data from ZIP archive containing Parquet file
% Extract to temporary directory
temp_dir = tempname;
unzip(zip_path, temp_dir);

% Load Parquet file (data.parquet)
parquet_path = fullfile(temp_dir, 'data.parquet');
if ~isfile(parquet_path)
    error('No data.parquet file found in ZIP archive');
end

% Read Parquet file (requires MATLAB R2019a+ or parquetread function)
try
    df_flat = parquetread(parquet_path);
catch
    error('Failed to read Parquet file. Ensure MATLAB R2019a+ or parquetread is available.');
end

% Identify meta columns vs sample columns
meta_cols = {'tx', 'rx', 'aq_number', 'tx_rx_id', 'log_version'};
all_cols = df_flat.Properties.VariableNames;
sample_cols = setdiff(all_cols, meta_cols, 'stable');

% Sort sample columns numerically (they're stored as strings like '0', '1', '2'...)
sample_nums = cellfun(@str2double, sample_cols);
[~, sort_idx] = sort(sample_nums);
sample_cols = sample_cols(sort_idx);

% Rebuild measurement column as cell array of vectors
n_rows = height(df_flat);
measurement_cell = cell(n_rows, 1);
for i = 1:n_rows
    measurement_cell{i} = table2array(df_flat(i, sample_cols))';
end

% Create output table
df = table();
df.measurement = measurement_cell;
df.tx = df_flat.tx;
df.rx = df_flat.rx;
df.aq_number = df_flat.aq_number;

if ismember('tx_rx_id', all_cols)
    df.tx_rx_id = df_flat.tx_rx_id;
else
    df.tx_rx_id = (0:n_rows-1)';
end

if ismember('log_version', all_cols)
    df.log_version = df_flat.log_version;
else
    df.log_version = ones(n_rows, 1);
end

% Convert index to datetime if available
try
    t = df_flat.x__index_level_0__;
    t = t / 1e6; % µs -> Sekunden
    df.time = datetime(t, 'ConvertFrom','posixtime', 'TimeZone','America/Vancouver');
catch
    df.time = (1:n_rows)';
end


% Load config JSON (config-0.json)
json_files = dir(fullfile(temp_dir, 'config-*.json'));
if ~isempty(json_files)
    json_path = fullfile(json_files(1).folder, json_files(1).name);
    config = jsondecode(fileread(json_path));
else
    config = struct();
end

% Cleanup
rmdir(temp_dir, 's');
end

function df_concat = concat_dataframes(df1, df2)
% Concatenate two dataframes
df_concat = [df1; df2];
end

function [flat_df, meas_cols] = flatten_df_measurements(df, sample_crop)
% Flatten measurement arrays into separate columns
n_rows = height(df);

if n_rows == 0
    flat_df = table();
    meas_cols = {};
    return;
end

% Get measurement dimensions
first_meas = df.measurement{1};
n_samples = min(length(first_meas), sample_crop);

% Pre-allocate matrix
meas_matrix = zeros(n_rows, n_samples);

for i = 1:n_rows
    meas = df.measurement{i};
    meas_matrix(i, :) = meas(1:n_samples);
end

% Create column names
meas_cols = arrayfun(@(x) sprintf('sample_%d', x-1), 1:n_samples, 'UniformOutput', false);

% Create flattened table
flat_df = [df(:, ~strcmp(df.Properties.VariableNames, 'measurement')), ...
    array2table(meas_matrix, 'VariableNames', meas_cols)];
end

function imshow_with_time_labels(timestamps)
% Set x-axis labels based on timestamps
n_acq = length(timestamps);
num_ticks = min(10, n_acq);

if n_acq > 0
    tick_positions = round(linspace(1, n_acq, num_ticks));
    xticks(tick_positions);

    % Format timestamps
    time_labels = cell(1, num_ticks);
    for i = 1:num_ticks
        idx = tick_positions(i);
        % Convert Unix timestamp to datetime
        dt = datetime(timestamps(idx), 'ConvertFrom', 'posixtime', 'Format', 'HH:mm:ss');
        time_labels{i} = char(dt);
    end
    xticklabels(time_labels);
    xtickangle(45);
end
end
