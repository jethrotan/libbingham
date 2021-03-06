function plot_pcd_clusters(pcd, options, B, W)
% plot_pcd_clusters(pcd, options) -- where options is a string
% contains any combination of the following characters:
%    'n'  --  plot normals
%    'p'  --  plot principal curvatures
%    'q'  --  plot quaternions


data = pcd.data;
columns = pcd.columns;

figure(1);
clf;
figure(2);
clf;

LINE_LENGTH_RATIO = .2; %.1;
LINE_SKIP_RATE = 5; %10;

plot_normals = 0;
plot_pcs = 0;
plot_quat = 0;
if nargin >= 2
   if strfind(options, 'n')
      plot_normals = 1;
   end
   if strfind(options, 'p')
      plot_pcs = 1;
      plot_normals = 1;
   end
   if strfind(options, 'q')
      plot_quat = 1;
      %plot_pcs = 1;
      %plot_normals = 1;
   end
end

plot_bmx = 0;
if nargin >= 3
    plot_bmx = 1;
end


% ch_cluster = find(strcmp(columns, 'cluster'));
% ch_x = find(strcmp(columns, 'x'));
% ch_y = find(strcmp(columns, 'y'));
% ch_z = find(strcmp(columns, 'z'));
% ch_pfh = find(strncmp(columns, 'f', 1));
% 
% if isempty(ch_cluster)
%    fprintf('Error: channel "cluster" not found.');
%    return
% end
% if isempty(ch_x)
%    fprintf('Error: channel "x" not found.');
%    return
% end
% if isempty(ch_y)
%    fprintf('Error: channel "y" not found.');
%    return
% end
% if isempty(ch_z)
%    fprintf('Error: channel "z" not found.');
%    return
% end
% if isempty(ch_pfh)
%    fprintf('Error: channels "f*" not found.');
%    return
% end

X = pcd.X;  %data(:, ch_x);
Y = pcd.Y;  %data(:, ch_y);
Z = pcd.Z;  %data(:, ch_z);
F = pcd.F;  %data(:, ch_pfh);
L = pcd.L;  %data(:, ch_cluster);
k = max(L)+1;

mean_std = mean([std(X) std(Y) std(Z)]);
line_length = LINE_LENGTH_RATIO * mean_std;

if plot_normals
%    ch_nx = find(strcmp(columns, 'nx'));
%    ch_ny = find(strcmp(columns, 'ny'));
%    ch_nz = find(strcmp(columns, 'nz'));
%    if isempty(ch_nx)
%       fprintf('Error: channel "nx" not found.');
%       return
%    end
%    if isempty(ch_ny)
%       fprintf('Error: channel "ny" not found.');
%       return
%    end
%    if isempty(ch_nz)
%       fprintf('Error: channel "nz" not found.');
%       return
%    end
   NX = pcd.NX;  %data(:, ch_nx);
   NY = pcd.NY;  %data(:, ch_ny);
   NZ = pcd.NZ;  %data(:, ch_nz);
end

if plot_pcs
%    ch_pcx = find(strcmp(columns, 'pcx'));
%    ch_pcy = find(strcmp(columns, 'pcy'));
%    ch_pcz = find(strcmp(columns, 'pcz'));
%    if isempty(ch_pcx)
%       fprintf('Error: channel "pcx" not found.');
%       return
%    end
%    if isempty(ch_pcy)
%       fprintf('Error: channel "pcy" not found.');
%       return
%    end
%    if isempty(ch_pcz)
%       fprintf('Error: channel "pcz" not found.');
%       return
%    end
   PCX = pcd.PCX;  %data(:, ch_pcx);
   PCY = pcd.PCY;  %data(:, ch_pcy);
   PCZ = pcd.PCZ;  %data(:, ch_pcz);
end

if plot_quat
   Q = pcd.Q;  %get_pcd_quaternions(data, columns);
end

for i=0:max(L)
   Li = find(L==i);

   figure(1);
   clf;
   
   % plot points
   plot3(X, Y, Z, '.', 'Color', [.9 .9 1]);
   hold on;
   
   % plot cluster
   plot3(X(Li), Y(Li), Z(Li), '.', 'Color', [1 0 0]);
   
   % plot normals
   if plot_normals
      for j=1:LINE_SKIP_RATE:length(Li)
         r = line_length;
         x = X(Li(j)); y = Y(Li(j)); z = Z(Li(j));
         nx = NX(Li(j)); ny = NY(Li(j)); nz = NZ(Li(j)); 
         plot3([x x+r*nx], [y y+r*ny], [z z+r*nz], 'k-', 'LineWidth', 2);
      end
   end
   
   % plot principal curvatures
   if plot_pcs
      r = line_length;
      for j=1:LINE_SKIP_RATE:length(Li)
         x = X(Li(j)); y = Y(Li(j)); z = Z(Li(j));
         pcx = PCX(Li(j)); pcy = PCY(Li(j)); pcz = PCZ(Li(j)); 
         plot3([x x+r*pcx], [y y+r*pcy], [z z+r*pcz], 'g-', 'LineWidth', 2);
      end
      for j=1:LINE_SKIP_RATE:length(Li)
         x = X(Li(j)); y = Y(Li(j)); z = Z(Li(j));
         pcx = PCX(Li(j)); pcy = PCY(Li(j)); pcz = PCZ(Li(j)); 
         nx = NX(Li(j)); ny = NY(Li(j)); nz = NZ(Li(j)); 
         pcx2 = ny*pcz - nz*pcy;
         pcy2 = nz*pcx - nx*pcz;
         pcz2 = nx*pcy - ny*pcx;
         plot3([x x+r*pcx2], [y y+r*pcy2], [z z+r*pcz2], 'm-', 'LineWidth', 2);
      end
   end
   hold off;
   axis vis3d;
   axis equal;
   %axis off;
   
   if plot_quat
      Qi = [Q(Li,:,1) ; Q(Li,:,2)];
      %[B_V B_Z B_F] = bingham_fit(Qi')

      %[V1 Z1 F1 V2 Z2 F2] = bingham_fit_bimodal(Qi')
%      Z1
%      Z2

      figure(3);
      %clf;
      %plot_bingham_3d(V1, Z1, F1);
      plot_quaternions(Qi);
      
      %figure(4);
      %plot_bingham_3d(V2, Z2, F2);
      %plot_quaternions(Qi);
      
      %figure(5);
      %clf;
      %subplot(2,1,1);
      %[SX,SY,SZ] = sphere(30);
      %surf(SX,SY,SZ, 'EdgeColor', 'none');
      %axis vis3d;
      %colormap(.5*gray+.5);
      %cmap = jet;
      %P1 = zeros(1, 2*length(Li));
      %P2 = zeros(1, 2*length(Li));
      %for j=1:2*length(Li)
      %   P1(j) = bingham_pdf(Qi(j,:), V1, Z1, F1);
      %   P2(j) = bingham_pdf(Qi(j,:), V2, Z2, F2);
      %end
      %P1 = P1./max(P1);
      %P2 = P2./max(P2);
      %C1 = cmap(round(1+63*P1), :);
      %C2 = cmap(round(1+63*P2), :);
      %plot_quaternions(Qi, C1);

      %figure(6);
      %clf;
      %subplot(2,1,1);
      %surf(SX,SY,SZ, 'EdgeColor', 'none');
      %axis vis3d;
      %colormap(.5*gray+.5);
      %plot_quaternions(Qi, C2);
      
      %figure(7);
      %plot_bingham_3d_projections(V1, Z1, F1);
      
      %figure(8);
      %plot_bingham_3d_projections(V2, Z2, F2);
      
      %figure(5);
      %A = 2*acos(Q(Li,:,1));
      %cmap = jet;
      %C = cmap(round(1+63*A/(2*pi)), :);
      %plot_quaternions(Q(Li,:,2), C);
   end


   
   figure(2);
   % plot pfh
   subplot(k,1,i+1);
   bar(mean(F(Li,:)), 'r');
   if i>0
      subplot(k,1,i);
      bar(mean(F(find(L==i-1),:)), 'b');
   end

   if plot_bmx
       figure(9);
       clf;
       for j=1:length(B{i+1})
           subplot(1,length(B{i+1}),j);
           plot_quaternions(bingham_sample(B{i+1}(j),200));
           title(sprintf('entropy = %f', bingham_entropy(B{i+1}(j))));
       end
       figure(10);
       clf;
       plot_quaternions(bingham_mixture_sample(B{i+1},W{i+1},500));
       title(sprintf('avg. entropy = %f', bingham_mixture_entropy(B{i+1}, W{i+1})));
   end

   
   fprintf('cluster %d', i);
   input(':');
end
figure(2);
subplot(k,1,k);
bar(mean(F(find(L==k-1),:)), 'b');



