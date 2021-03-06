function pcd = populate_pcd_fields(columns, data)
% pcd = populate_pcd_fields(columns, data) -- populate the fields of a pcd

pcd.columns = columns;
pcd.data = data;

ch_cluster = find(strcmp(columns, 'cluster'));
ch_x = find(strcmp(columns, 'x'));
ch_y = find(strcmp(columns, 'y'));
ch_z = find(strcmp(columns, 'z'));
ch_vx = find(strcmp(columns, 'vx'));
ch_vy = find(strcmp(columns, 'vy'));
ch_vz = find(strcmp(columns, 'vz'));
ch_pfh = find(strncmp(columns, 'f', 1));
ch_pfh_small = find(strncmp(columns, 'sf', 2));
ch_sift = find(strncmp(columns, 'sift', 4));
ch_nx = find(strcmp(columns, 'nx'));
ch_ny = find(strcmp(columns, 'ny'));
ch_nz = find(strcmp(columns, 'nz'));
ch_curv = find(strcmp(columns, 'curvature'));
ch_pcx = find(strcmp(columns, 'pcx'));
ch_pcy = find(strcmp(columns, 'pcy'));
ch_pcz = find(strcmp(columns, 'pcz'));
ch_pc1 = find(strcmp(columns, 'pc1'));
ch_pc2 = find(strcmp(columns, 'pc2'));
ch_red = find(strcmp(columns, 'red'));
ch_green = find(strcmp(columns, 'green'));
ch_blue = find(strcmp(columns, 'blue'));
ch_balls = find(strcmp(columns, 'balls'));
ch_segments = find(strcmp(columns, 'segments'));
ch_surfdist = find(strcmp(columns, 'surfdist'));
ch_surfwidth = find(strcmp(columns, 'surfwidth'));
ch_normalvar = find(strcmp(columns, 'normalvar'));

% labdist
% ch_m = find(strncmp(columns, 'm', 1));
% ch_cov = find(strncmp(columns, 'cov', 3));
% 
% ch_cnt = find(strncmp(columns, 'cnt', 3));

if ~isempty(ch_cluster)
   pcd.L = data(:, ch_cluster);
   pcd.k = max(pcd.L)+1;
end
if ~isempty(ch_x)
   pcd.X = data(:, ch_x);
end
if ~isempty(ch_y)
   pcd.Y = data(:, ch_y);
end
if ~isempty(ch_z)
   pcd.Z = data(:, ch_z);
end
if ~isempty(ch_vx)
   pcd.VX = data(:, ch_vx);
end
if ~isempty(ch_vy)
   pcd.VY = data(:, ch_vy);
end
if ~isempty(ch_vz)
   pcd.VZ = data(:, ch_vz);
end
if ~isempty(ch_pfh)
   pcd.F = data(:, ch_pfh);
end
if ~isempty(ch_pfh_small)
   pcd.F_small = data(:, ch_pfh_small);
end
if ~isempty(ch_sift)
   pcd.SIFT = data(:, ch_sift);
end
if ~isempty(ch_nx)
   pcd.NX = data(:, ch_nx);
end
if ~isempty(ch_ny)
   pcd.NY = data(:, ch_ny);
end
if ~isempty(ch_nz)
   pcd.NZ = data(:, ch_nz);
end
if ~isempty(ch_curv)
    pcd.C = data(:, ch_curv);
end
if ~isempty(ch_pcx)
   pcd.PCX = data(:, ch_pcx);
end
if ~isempty(ch_pcy)
   pcd.PCY = data(:, ch_pcy);
end
if ~isempty(ch_pcz)
   pcd.PCZ = data(:, ch_pcz);
end
if ~isempty(ch_pc1)
   pcd.PC1 = data(:, ch_pc1);
end
if ~isempty(ch_pc2)
   pcd.PC2 = data(:, ch_pc2);
end
if ~isempty(ch_nx) && ~isempty(ch_pcx)
   pcd.Q = get_pcd_quaternions(pcd.data, pcd.columns);
end
if ~isempty(ch_pfh) && ~isempty(ch_cluster)
    pcd.M = zeros(pcd.k, size(pcd.F,2));
    pcd.V = zeros(1, pcd.k);
    for i=1:pcd.k
        pcd.M(i,:) = mean(pcd.F(pcd.L==i-1,:));
        pcd.V(i) = sum(var(pcd.F(pcd.L==i-1,:)));
    end
end
if ~isempty(ch_red)
   pcd.R = data(:, ch_red);
end
if ~isempty(ch_green)
   pcd.G = data(:, ch_green);
end
if ~isempty(ch_blue)
   pcd.B = data(:, ch_blue);
end
if ~isempty(ch_balls)
   pcd.balls = data(:, ch_balls);
end
if ~isempty(ch_segments)
   pcd.segments = data(:, ch_segments);
end
if ~isempty(ch_surfdist)
   pcd.surfdist = data(:, ch_surfdist);
end
if ~isempty(ch_surfwidth)
   pcd.surfwidth = data(:, ch_surfwidth);
end
if ~isempty(ch_normalvar)
   pcd.normalvar = data(:, ch_normalvar);
end

% if ~isempty(ch_m)
%    pcd.M1 = data(:, ch_m(1:3));
%    pcd.M2 = data(:, ch_m(4:6));   
%    tmp_cov1 = data(:, ch_cov(1:6));
%    tmp_cov2 = data(:, ch_cov(7:12));
%    pcd.C1 = tmp_cov1(:, [1, 2, 3, 2, 4, 5, 3, 5, 6]); % This is awful...
%    pcd.C2 = tmp_cov2(:, [1, 2, 3, 2, 4, 5, 3, 5, 6]); % This is awful...
%    pcd.CNT1 = data(:, ch_cnt(1));
%    pcd.CNT2 = data(:, ch_cnt(2));
% end
