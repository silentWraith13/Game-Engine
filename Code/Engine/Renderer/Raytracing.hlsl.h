#if 0
;
; Note: shader requires additional functionality :
;       UAVs at every shader stage
;
; shader debug name : 83944734f49977c3bf3b2aba8d7a56f1.pdb
; shader hash : 83944734f49977c3bf3b2aba8d7a56f1
;
; Buffer Definitions :
;
; cbuffer g_rayGenCB
; {
	;
	;   struct g_rayGenCB
		; {
		;
		;       struct struct.RayGenConstantBuffer
			; {
			;
			;           struct struct.Viewport
				; {
				;
				;               float left; ; Offset:    0
					;               float top; ; Offset:    4
					;               float right; ; Offset:    8
					;               float bottom; ; Offset:   12
					;
				;           } viewport; ; Offset:    0
				;
				;           struct struct.Viewport
					; {
					;
					;               float left; ; Offset:   16
						;               float top; ; Offset:   20
						;               float right; ; Offset:   24
						;               float bottom; ; Offset:   28
						;
					;           } stencil; ; Offset:   16
					;
					;
					;       } g_rayGenCB; ; Offset:    0
			;
					;
					;   } g_rayGenCB; ; Offset:    0 Size : 32
		;
					; }
;
;
; Resource Bindings :
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------ - ---------- - ------ - -------------- ------
; g_rayGenCB                        cbuffer      NA          NA     CB0            cb0     1
; Scene                             texture     i32         ras      T0             t0     1
; RenderTarget                          UAV     f32          2d      U0             u0     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

% struct.RaytracingAccelerationStructure = type{ i32 }
% "class.RWTexture2D<vector<float, 4> >" = type{ <4 x float> }
% g_rayGenCB = type{ % struct.RayGenConstantBuffer }
% struct.RayGenConstantBuffer = type{ % struct.Viewport,% struct.Viewport }
% struct.Viewport = type{ float, float, float, float }
% dx.types.Handle = type{ i8* }
% struct.RayPayload = type{ <4 x float> }
% struct.RayDesc = type{ <3 x float>, float, <3 x float>, float }
% dx.types.CBufRet.f32 = type{ float, float, float, float }
% struct.BuiltInTriangleIntersectionAttributes = type{ <2 x float> }

@"\01?Scene@@3URaytracingAccelerationStructure@@A" = external constant % struct.RaytracingAccelerationStructure, align 4
@"\01?RenderTarget@@3V?$RWTexture2D@V?$vector@M$03@@@@A" = external constant % "class.RWTexture2D<vector<float, 4> >", align 4
@g_rayGenCB = external constant % g_rayGenCB

; Function Attrs : nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

; Function Attrs : nounwind
define void @"\01?MyRaygenShader@@YAXXZ"() #1 {
	% 1 = alloca[4 x float]
		% 2 = alloca i1, align 4
		% 3 = alloca[2 x float]
		% 4 = load % struct.RaytracingAccelerationStructure, % struct.RaytracingAccelerationStructure * @"\01?Scene@@3URaytracingAccelerationStructure@@A"
		% 5 = load % "class.RWTexture2D<vector<float, 4> >", % "class.RWTexture2D<vector<float, 4> >" * @"\01?RenderTarget@@3V?$RWTexture2D@V?$vector@M$03@@@@A"
		% 6 = load % g_rayGenCB, % g_rayGenCB * @g_rayGenCB
		% g_rayGenCB = call % dx.types.Handle @dx.op.createHandleForLib.g_rayGenCB(i32 160, % g_rayGenCB % 6); CreateHandleForLib(Resource)
		% 7 = alloca % struct.RayPayload
		% 8 = alloca[2 x float]
		% 9 = alloca[3 x float]
		% 10 = alloca[3 x float]
		% ray = alloca % struct.RayDesc, align 4
		% 11 = alloca[3 x i32]
		% 12 = alloca[3 x i32]
		call void @llvm.dbg.declare(metadata[2 x float] * %8, metadata !129, metadata !130), !dbg !131
		% DispatchRaysIndex = call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 0), !dbg !132; DispatchRaysIndex(col)
		% DispatchRaysIndex3 = call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 1), !dbg !132; DispatchRaysIndex(col)
		% .i0 = uitofp i32 % DispatchRaysIndex to float, !dbg !132
		% .i1 = uitofp i32 % DispatchRaysIndex3 to float, !dbg !132
		% DispatchRaysDimensions = call i32 @dx.op.dispatchRaysDimensions.i32(i32 146, i8 0), !dbg !133; DispatchRaysDimensions(col)
		% DispatchRaysDimensions1 = call i32 @dx.op.dispatchRaysDimensions.i32(i32 146, i8 1), !dbg !133; DispatchRaysDimensions(col)
		% .i011 = uitofp i32 % DispatchRaysDimensions to float, !dbg !133
		% .i112 = uitofp i32 % DispatchRaysDimensions1 to float, !dbg !133
		% .i013 = fdiv fast float% .i0, % .i011, !dbg !134
		% .i114 = fdiv fast float% .i1, % .i112, !dbg !134
		% 13 = getelementptr inbounds[2 x float], [2 x float] * %8, i32 0, i32 0, !dbg !131
		store float% .i013, float*% 13, !dbg !131
		% 14 = getelementptr inbounds[2 x float], [2 x float] * %8, i32 0, i32 1, !dbg !131
		store float% .i114, float*% 14, !dbg !131
		call void @llvm.dbg.declare(metadata[3 x float] * %9, metadata !135, metadata !130), !dbg !136
		% 15 = getelementptr inbounds[3 x float], [3 x float] * %9, i32 0, i32 0, !dbg !136
		store float 0.000000e+00, float*% 15, !dbg !136
		% 16 = getelementptr inbounds[3 x float], [3 x float] * %9, i32 0, i32 1, !dbg !136
		store float 0.000000e+00, float*% 16, !dbg !136
		% 17 = getelementptr inbounds[3 x float], [3 x float] * %9, i32 0, i32 2, !dbg !136
		store float 1.000000e+00, float*% 17, !dbg !136
		call void @llvm.dbg.declare(metadata[3 x float] * %10, metadata !137, metadata !130), !dbg !138
		% 18 = getelementptr inbounds[2 x float], [2 x float] * %8, i32 0, i32 0, !dbg !139
		% 19 = load float, float*% 18, !dbg !139
		% 20 = call % dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, % dx.types.Handle % g_rayGenCB, i32 0), !dbg !140; CBufferLoadLegacy(handle, regIndex)
		% 21 = extractvalue % dx.types.CBufRet.f32 % 20, 2, !dbg !140
		% 22 = call % dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, % dx.types.Handle % g_rayGenCB, i32 0), !dbg !141; CBufferLoadLegacy(handle, regIndex)
		% 23 = extractvalue % dx.types.CBufRet.f32 % 22, 0, !dbg !141
		% 24 = fsub fast float% 21, % 23, !dbg !142
		% 25 = fmul fast float% 19, % 24, !dbg !142
		% 26 = fadd fast float% 23, % 25, !dbg !142
		% 27 = getelementptr inbounds[2 x float], [2 x float] * %8, i32 0, i32 1, !dbg !143
		% 28 = load float, float*% 27, !dbg !143
		% 29 = call % dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, % dx.types.Handle % g_rayGenCB, i32 0), !dbg !144; CBufferLoadLegacy(handle, regIndex)
		% 30 = extractvalue % dx.types.CBufRet.f32 % 29, 3, !dbg !144
		% 31 = call % dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, % dx.types.Handle % g_rayGenCB, i32 0), !dbg !145; CBufferLoadLegacy(handle, regIndex)
		% 32 = extractvalue % dx.types.CBufRet.f32 % 31, 1, !dbg !145
		% 33 = fsub fast float% 30, % 32, !dbg !146
		% 34 = fmul fast float% 28, % 33, !dbg !146
		% 35 = fadd fast float% 32, % 34, !dbg !146
		% 36 = getelementptr inbounds[3 x float], [3 x float] * %10, i32 0, i32 0, !dbg !138
		store float% 26, float*% 36, !dbg !138
		% 37 = getelementptr inbounds[3 x float], [3 x float] * %10, i32 0, i32 1, !dbg !138
		store float% 35, float*% 37, !dbg !138
		% 38 = getelementptr inbounds[3 x float], [3 x float] * %10, i32 0, i32 2, !dbg !138
		store float 0.000000e+00, float*% 38, !dbg !138
		% 39 = getelementptr inbounds[3 x float], [3 x float] * %10, i32 0, i32 0, !dbg !147
		% 40 = load float, float*% 39, !dbg !147
		% 41 = getelementptr inbounds[3 x float], [3 x float] * %10, i32 0, i32 1, !dbg !147
		% 42 = load float, float*% 41, !dbg !147
		% 43 = getelementptr inbounds[2 x float], [2 x float] * %3, i32 0, i32 0, !dbg !149
		store float% 40, float*% 43, !dbg !149
		% 44 = getelementptr inbounds[2 x float], [2 x float] * %3, i32 0, i32 1, !dbg !149
		store float% 42, float*% 44, !dbg !149
		% 45 = getelementptr inbounds[2 x float], [2 x float] * %3, i32 0, i32 0, !dbg !150
		% 46 = load float, float*% 45, !dbg !150
		% 47 = call % dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, % dx.types.Handle % g_rayGenCB, i32 1), !dbg !152; CBufferLoadLegacy(handle, regIndex)
		% 48 = extractvalue % dx.types.CBufRet.f32 % 47, 0, !dbg !152
		% 49 = fcmp fast oge float% 46, % 48, !dbg !153
		% 50 = getelementptr inbounds[2 x float], [2 x float] * %3, i32 0, i32 0, !dbg !154
		% 51 = load float, float*% 50, !dbg !154
		% 52 = call % dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, % dx.types.Handle % g_rayGenCB, i32 1), !dbg !155; CBufferLoadLegacy(handle, regIndex)
		% 53 = extractvalue % dx.types.CBufRet.f32 % 52, 2, !dbg !155
		% 54 = fcmp fast ole float% 51, % 53, !dbg !156
		% 55 = and i1 % 49, % 54, !dbg !157
		% 56 = getelementptr inbounds[2 x float], [2 x float] * %3, i32 0, i32 1, !dbg !158
		% 57 = load float, float*% 56, !dbg !158
		% 58 = call % dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, % dx.types.Handle % g_rayGenCB, i32 1), !dbg !159; CBufferLoadLegacy(handle, regIndex)
		% 59 = extractvalue % dx.types.CBufRet.f32 % 58, 1, !dbg !159
		% 60 = fcmp fast oge float% 57, % 59, !dbg !160
		% 61 = getelementptr inbounds[2 x float], [2 x float] * %3, i32 0, i32 1, !dbg !161
		% 62 = load float, float*% 61, !dbg !161
		% 63 = call % dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, % dx.types.Handle % g_rayGenCB, i32 1), !dbg !162; CBufferLoadLegacy(handle, regIndex)
		% 64 = extractvalue % dx.types.CBufRet.f32 % 63, 3, !dbg !162
		% 65 = fcmp fast ole float% 62, % 64, !dbg !163
		% 66 = and i1 % 60, % 65, !dbg !164
		% 67 = and i1 % 55, % 66, !dbg !165
		store i1 % 67, i1*% 2, !dbg !166
		% 68 = load i1, i1 * %2, !dbg !166
		call void @llvm.dbg.declare(metadata[2 x float] * %3, metadata !167, metadata !130), !dbg !168
		br i1 % 68, label % 69, label % 150, !dbg !169

		; <label>:69; preds = % 0
		call void @llvm.dbg.declare(metadata % struct.RayDesc * %ray, metadata !170, metadata !130), !dbg !178
		% 70 = getelementptr inbounds[3 x float], [3 x float] * %10, i32 0, i32 0, !dbg !179
		% 71 = load float, float*% 70, !dbg !179
		% 72 = insertelement <3 x float> undef, float% 71, i64 0, !dbg !179
		% 73 = getelementptr inbounds[3 x float], [3 x float] * %10, i32 0, i32 1, !dbg !179
		% 74 = load float, float*% 73, !dbg !179
		% 75 = insertelement <3 x float> % 72, float% 74, i64 1, !dbg !179
		% 76 = getelementptr inbounds[3 x float], [3 x float] * %10, i32 0, i32 2, !dbg !179
		% 77 = load float, float*% 76, !dbg !179
		% 78 = insertelement <3 x float> % 75, float% 77, i64 2, !dbg !179
		% 79 = getelementptr inbounds % struct.RayDesc, % struct.RayDesc * %ray, i32 0, i32 0, !dbg !180
		store <3 x float> % 78, <3 x float>*%79, align 4, !dbg !181
		% 80 = getelementptr inbounds[3 x float], [3 x float] * %9, i32 0, i32 0, !dbg !182
		% 81 = load float, float*% 80, !dbg !182
		% 82 = insertelement <3 x float> undef, float% 81, i64 0, !dbg !182
		% 83 = getelementptr inbounds[3 x float], [3 x float] * %9, i32 0, i32 1, !dbg !182
		% 84 = load float, float*% 83, !dbg !182
		% 85 = insertelement <3 x float> % 82, float% 84, i64 1, !dbg !182
		% 86 = getelementptr inbounds[3 x float], [3 x float] * %9, i32 0, i32 2, !dbg !182
		% 87 = load float, float*% 86, !dbg !182
		% 88 = insertelement <3 x float> % 85, float% 87, i64 2, !dbg !182
		% 89 = getelementptr inbounds % struct.RayDesc, % struct.RayDesc * %ray, i32 0, i32 2, !dbg !183
		store <3 x float> % 88, <3 x float>*%89, align 4, !dbg !184
		% 90 = getelementptr inbounds % struct.RayDesc, % struct.RayDesc * %ray, i32 0, i32 1, !dbg !185
		store float 0x3F50624DE0000000, float*% 90, align 4, !dbg !186
		% 91 = getelementptr inbounds % struct.RayDesc, % struct.RayDesc * %ray, i32 0, i32 3, !dbg !187
		store float 1.000000e+04, float*% 91, align 4, !dbg !188
		call void @llvm.dbg.declare(metadata[4 x float] * %1, metadata !189, metadata !130), !dbg !190
		% 92 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 0, !dbg !191
		store float 0.000000e+00, float*% 92, !dbg !191
		% 93 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 1, !dbg !191
		store float 0.000000e+00, float*% 93, !dbg !191
		% 94 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 2, !dbg !191
		store float 0.000000e+00, float*% 94, !dbg !191
		% 95 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 3, !dbg !191
		store float 0.000000e+00, float*% 95, !dbg !191
		% 96 = getelementptr inbounds % struct.RayPayload, % struct.RayPayload * %7, i32 0, i32 0, !dbg !192
		% 97 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 0, !dbg !192
		% 98 = load float, float*% 97, !dbg !192
		% 99 = insertelement <4 x float> undef, float% 98, i64 0, !dbg !192
		% 100 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 1, !dbg !192
		% 101 = load float, float*% 100, !dbg !192
		% 102 = insertelement <4 x float> % 99, float% 101, i64 1, !dbg !192
		% 103 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 2, !dbg !192
		% 104 = load float, float*% 103, !dbg !192
		% 105 = insertelement <4 x float> % 102, float% 104, i64 2, !dbg !192
		% 106 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 3, !dbg !192
		% 107 = load float, float*% 106, !dbg !192
		% 108 = insertelement <4 x float> % 105, float% 107, i64 3, !dbg !192
		store <4 x float> % 108, <4 x float>*%96, !dbg !192
		% 109 = call % dx.types.Handle @dx.op.createHandleForLib.struct.RaytracingAccelerationStructure(i32 160, % struct.RaytracingAccelerationStructure % 4), !dbg !192; CreateHandleForLib(Resource)
		% 110 = getelementptr % struct.RayDesc, % struct.RayDesc * %ray, i32 0, i32 0, !dbg !192
		% 111 = load <3 x float>, <3 x float>*%110, !dbg !192
		% 112 = extractelement <3 x float> % 111, i64 0, !dbg !192
		% 113 = extractelement <3 x float> % 111, i64 1, !dbg !192
		% 114 = extractelement <3 x float> % 111, i64 2, !dbg !192
		% 115 = getelementptr % struct.RayDesc, % struct.RayDesc * %ray, i32 0, i32 1, !dbg !192
		% 116 = load float, float*% 115, !dbg !192
		% 117 = getelementptr % struct.RayDesc, % struct.RayDesc * %ray, i32 0, i32 2, !dbg !192
		% 118 = load <3 x float>, <3 x float>*%117, !dbg !192
		% 119 = extractelement <3 x float> % 118, i64 0, !dbg !192
		% 120 = extractelement <3 x float> % 118, i64 1, !dbg !192
		% 121 = extractelement <3 x float> % 118, i64 2, !dbg !192
		% 122 = getelementptr % struct.RayDesc, % struct.RayDesc * %ray, i32 0, i32 3, !dbg !192
		% 123 = load float, float*% 122, !dbg !192
		call void @dx.op.traceRay.struct.RayPayload(i32 157, % dx.types.Handle % 109, i32 16, i32 - 1, i32 0, i32 1, i32 0, float% 112, float% 113, float% 114, float% 116, float% 119, float% 120, float% 121, float% 123, % struct.RayPayload * %7), !dbg !192; TraceRay(AccelerationStructure, RayFlags, InstanceInclusionMask, RayContributionToHitGroupIndex, MultiplierForGeometryContributionToShaderIndex, MissShaderIndex, Origin_X, Origin_Y, Origin_Z, TMin, Direction_X, Direction_Y, Direction_Z, TMax, payload)
		% 124 = getelementptr inbounds % struct.RayPayload, % struct.RayPayload * %7, i32 0, i32 0, !dbg !192
		% 125 = load <4 x float>, <4 x float>*%124, !dbg !192
		% 126 = extractelement <4 x float> % 125, i64 0, !dbg !192
		% 127 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 0, !dbg !192
		store float% 126, float*% 127, !dbg !192
		% 128 = extractelement <4 x float> % 125, i64 1, !dbg !192
		% 129 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 1, !dbg !192
		store float% 128, float*% 129, !dbg !192
		% 130 = extractelement <4 x float> % 125, i64 2, !dbg !192
		% 131 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 2, !dbg !192
		store float% 130, float*% 131, !dbg !192
		% 132 = extractelement <4 x float> % 125, i64 3, !dbg !192
		% 133 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 3, !dbg !192
		store float% 132, float*% 133, !dbg !192
		% 134 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 0, !dbg !193
		% 135 = load float, float*% 134, !dbg !193
		% 136 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 1, !dbg !193
		% 137 = load float, float*% 136, !dbg !193
		% 138 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 2, !dbg !193
		% 139 = load float, float*% 138, !dbg !193
		% 140 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 3, !dbg !193
		% 141 = load float, float*% 140, !dbg !193
		% DispatchRaysIndex5 = call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 0), !dbg !194; DispatchRaysIndex(col)
		% DispatchRaysIndex6 = call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 1), !dbg !194; DispatchRaysIndex(col)
		% DispatchRaysIndex7 = call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 2), !dbg !194; DispatchRaysIndex(col)
		% 142 = getelementptr inbounds[3 x i32], [3 x i32] * %11, i32 0, i32 0, !dbg !194
		store i32 % DispatchRaysIndex5, i32*% 142, !dbg !194
		% 143 = getelementptr inbounds[3 x i32], [3 x i32] * %11, i32 0, i32 1, !dbg !194
		store i32 % DispatchRaysIndex6, i32*% 143, !dbg !194
		% 144 = getelementptr inbounds[3 x i32], [3 x i32] * %11, i32 0, i32 2, !dbg !194
		store i32 % DispatchRaysIndex7, i32*% 144, !dbg !194
		% 145 = getelementptr inbounds[3 x i32], [3 x i32] * %11, i32 0, i32 0, !dbg !194
		% 146 = load i32, i32 * %145, !dbg !194
		% 147 = getelementptr inbounds[3 x i32], [3 x i32] * %11, i32 0, i32 1, !dbg !194
		% 148 = load i32, i32 * %147, !dbg !194
		% 149 = call % dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2D<vector<float, 4> >"(i32 160, % "class.RWTexture2D<vector<float, 4> >" % 5), !dbg !195; CreateHandleForLib(Resource)
		call void @dx.op.textureStore.f32(i32 67, % dx.types.Handle % 149, i32 % 146, i32 % 148, i32 undef, float% 135, float% 137, float% 139, float% 141, i8 15), !dbg !196; TextureStore(srv, coord0, coord1, coord2, value0, value1, value2, value3, mask)
		br label % 163, !dbg !197

		; <label>:150; preds = % 0
		% 151 = getelementptr inbounds[2 x float], [2 x float] * %8, i32 0, i32 0, !dbg !198
		% 152 = load float, float*% 151, !dbg !198
		% 153 = getelementptr inbounds[2 x float], [2 x float] * %8, i32 0, i32 1, !dbg !198
		% 154 = load float, float*% 153, !dbg !198
		% DispatchRaysIndex8 = call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 0), !dbg !200; DispatchRaysIndex(col)
		% DispatchRaysIndex9 = call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 1), !dbg !200; DispatchRaysIndex(col)
		% DispatchRaysIndex10 = call i32 @dx.op.dispatchRaysIndex.i32(i32 145, i8 2), !dbg !200; DispatchRaysIndex(col)
		% 155 = getelementptr inbounds[3 x i32], [3 x i32] * %12, i32 0, i32 0, !dbg !200
		store i32 % DispatchRaysIndex8, i32*% 155, !dbg !200
		% 156 = getelementptr inbounds[3 x i32], [3 x i32] * %12, i32 0, i32 1, !dbg !200
		store i32 % DispatchRaysIndex9, i32*% 156, !dbg !200
		% 157 = getelementptr inbounds[3 x i32], [3 x i32] * %12, i32 0, i32 2, !dbg !200
		store i32 % DispatchRaysIndex10, i32*% 157, !dbg !200
		% 158 = getelementptr inbounds[3 x i32], [3 x i32] * %12, i32 0, i32 0, !dbg !200
		% 159 = load i32, i32 * %158, !dbg !200
		% 160 = getelementptr inbounds[3 x i32], [3 x i32] * %12, i32 0, i32 1, !dbg !200
		% 161 = load i32, i32 * %160, !dbg !200
		% 162 = call % dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2D<vector<float, 4> >"(i32 160, % "class.RWTexture2D<vector<float, 4> >" % 5), !dbg !201; CreateHandleForLib(Resource)
		call void @dx.op.textureStore.f32(i32 67, % dx.types.Handle % 162, i32 % 159, i32 % 161, i32 undef, float% 152, float% 154, float 0.000000e+00, float 1.000000e+00, i8 15), !dbg !202; TextureStore(srv, coord0, coord1, coord2, value0, value1, value2, value3, mask)
		br label % 163

		; <label>:163; preds = % 150, % 69
		ret void, !dbg !203
}

; Function Attrs : nounwind
define void @"\01?MyClosestHitShader@@YAXURayPayload@@UBuiltInTriangleIntersectionAttributes@@@Z"(% struct.RayPayload * noalias % payload, % struct.BuiltInTriangleIntersectionAttributes * %attr) #1 {
	% 1 = alloca[4 x float]
		% 2 = alloca[3 x float]
		call void @llvm.dbg.declare(metadata % struct.BuiltInTriangleIntersectionAttributes * %attr, metadata !204, metadata !205), !dbg !206
		call void @llvm.dbg.declare(metadata[4 x float] * %1, metadata !207, metadata !130), !dbg !208
		call void @llvm.dbg.declare(metadata[3 x float] * %2, metadata !209, metadata !130), !dbg !210
		% 3 = getelementptr inbounds % struct.RayPayload, % struct.RayPayload * %payload, i32 0, i32 0, !dbg !211
		% 4 = load <4 x float>, <4 x float>*%3, !dbg !211
		% 5 = extractelement <4 x float> % 4, i64 0, !dbg !211
		% 6 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 0, !dbg !211
		store float% 5, float*% 6, !dbg !211
		% 7 = extractelement <4 x float> % 4, i64 1, !dbg !211
		% 8 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 1, !dbg !211
		store float% 7, float*% 8, !dbg !211
		% 9 = extractelement <4 x float> % 4, i64 2, !dbg !211
		% 10 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 2, !dbg !211
		store float% 9, float*% 10, !dbg !211
		% 11 = extractelement <4 x float> % 4, i64 3, !dbg !211
		% 12 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 3, !dbg !211
		store float% 11, float*% 12, !dbg !211
		% 13 = getelementptr inbounds % struct.BuiltInTriangleIntersectionAttributes, % struct.BuiltInTriangleIntersectionAttributes * %attr, i32 0, i32 0, !dbg !211
		% 14 = load <2 x float>, <2 x float>*%13, align 4, !dbg !212
		% 15 = extractelement <2 x float> % 14, i32 0, !dbg !212
		% 16 = fsub fast float 1.000000e+00, % 15, !dbg !213
		% 17 = getelementptr inbounds % struct.BuiltInTriangleIntersectionAttributes, % struct.BuiltInTriangleIntersectionAttributes * %attr, i32 0, i32 0, !dbg !214
		% 18 = load <2 x float>, <2 x float>*%17, align 4, !dbg !215
		% 19 = extractelement <2 x float> % 18, i32 1, !dbg !215
		% 20 = fsub fast float% 16, % 19, !dbg !216
		% 21 = getelementptr inbounds % struct.BuiltInTriangleIntersectionAttributes, % struct.BuiltInTriangleIntersectionAttributes * %attr, i32 0, i32 0, !dbg !217
		% 22 = load <2 x float>, <2 x float>*%21, align 4, !dbg !218
		% 23 = extractelement <2 x float> % 22, i32 0, !dbg !218
		% 24 = getelementptr inbounds % struct.BuiltInTriangleIntersectionAttributes, % struct.BuiltInTriangleIntersectionAttributes * %attr, i32 0, i32 0, !dbg !219
		% 25 = load <2 x float>, <2 x float>*%24, align 4, !dbg !220
		% 26 = extractelement <2 x float> % 25, i32 1, !dbg !220
		% 27 = getelementptr inbounds[3 x float], [3 x float] * %2, i32 0, i32 0, !dbg !210
		store float% 20, float*% 27, !dbg !210
		% 28 = getelementptr inbounds[3 x float], [3 x float] * %2, i32 0, i32 1, !dbg !210
		store float% 23, float*% 28, !dbg !210
		% 29 = getelementptr inbounds[3 x float], [3 x float] * %2, i32 0, i32 2, !dbg !210
		store float% 26, float*% 29, !dbg !210
		% 30 = getelementptr inbounds[3 x float], [3 x float] * %2, i32 0, i32 0, !dbg !221
		% 31 = load float, float*% 30, !dbg !221
		% 32 = getelementptr inbounds[3 x float], [3 x float] * %2, i32 0, i32 1, !dbg !221
		% 33 = load float, float*% 32, !dbg !221
		% 34 = getelementptr inbounds[3 x float], [3 x float] * %2, i32 0, i32 2, !dbg !221
		% 35 = load float, float*% 34, !dbg !221
		% 36 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 0, !dbg !222
		store float% 31, float*% 36, !dbg !222
		% 37 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 1, !dbg !222
		store float% 33, float*% 37, !dbg !222
		% 38 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 2, !dbg !222
		store float% 35, float*% 38, !dbg !222
		% 39 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 3, !dbg !222
		store float 1.000000e+00, float*% 39, !dbg !222
		% 40 = getelementptr inbounds % struct.RayPayload, % struct.RayPayload * %payload, i32 0, i32 0, !dbg !223
		% 41 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 0, !dbg !223
		% 42 = load float, float*% 41, !dbg !223
		% 43 = insertelement <4 x float> undef, float% 42, i64 0, !dbg !223
		% 44 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 1, !dbg !223
		% 45 = load float, float*% 44, !dbg !223
		% 46 = insertelement <4 x float> % 43, float% 45, i64 1, !dbg !223
		% 47 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 2, !dbg !223
		% 48 = load float, float*% 47, !dbg !223
		% 49 = insertelement <4 x float> % 46, float% 48, i64 2, !dbg !223
		% 50 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 3, !dbg !223
		% 51 = load float, float*% 50, !dbg !223
		% 52 = insertelement <4 x float> % 49, float% 51, i64 3, !dbg !223
		store <4 x float> % 52, <4 x float>*%40, !dbg !223
		ret void, !dbg !223
}

; Function Attrs : nounwind
define void @"\01?MyMissShader@@YAXURayPayload@@@Z"(% struct.RayPayload * noalias % payload) #1 {
	% 1 = alloca[4 x float], !dbg !224
		call void @llvm.dbg.declare(metadata[4 x float] * %1, metadata !225, metadata !130), !dbg !224
		% 2 = getelementptr inbounds % struct.RayPayload, % struct.RayPayload * %payload, i32 0, i32 0, !dbg !226
		% 3 = load <4 x float>, <4 x float>*%2, !dbg !226
		% 4 = extractelement <4 x float> % 3, i64 0, !dbg !226
		% 5 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 0, !dbg !226
		store float% 4, float*% 5, !dbg !226
		% 6 = extractelement <4 x float> % 3, i64 1, !dbg !226
		% 7 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 1, !dbg !226
		store float% 6, float*% 7, !dbg !226
		% 8 = extractelement <4 x float> % 3, i64 2, !dbg !226
		% 9 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 2, !dbg !226
		store float% 8, float*% 9, !dbg !226
		% 10 = extractelement <4 x float> % 3, i64 3, !dbg !226
		% 11 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 3, !dbg !226
		store float% 10, float*% 11, !dbg !226
		% 12 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 0, !dbg !227
		store float 0.000000e+00, float*% 12, !dbg !227
		% 13 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 1, !dbg !227
		store float 0.000000e+00, float*% 13, !dbg !227
		% 14 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 2, !dbg !227
		store float 0.000000e+00, float*% 14, !dbg !227
		% 15 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 3, !dbg !227
		store float 1.000000e+00, float*% 15, !dbg !227
		% 16 = getelementptr inbounds % struct.RayPayload, % struct.RayPayload * %payload, i32 0, i32 0, !dbg !228
		% 17 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 0, !dbg !228
		% 18 = load float, float*% 17, !dbg !228
		% 19 = insertelement <4 x float> undef, float% 18, i64 0, !dbg !228
		% 20 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 1, !dbg !228
		% 21 = load float, float*% 20, !dbg !228
		% 22 = insertelement <4 x float> % 19, float% 21, i64 1, !dbg !228
		% 23 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 2, !dbg !228
		% 24 = load float, float*% 23, !dbg !228
		% 25 = insertelement <4 x float> % 22, float% 24, i64 2, !dbg !228
		% 26 = getelementptr inbounds[4 x float], [4 x float] * %1, i32 0, i32 3, !dbg !228
		% 27 = load float, float*% 26, !dbg !228
		% 28 = insertelement <4 x float> % 25, float% 27, i64 3, !dbg !228
		store <4 x float> % 28, <4 x float>*%16, !dbg !228
		ret void, !dbg !228
}

; Function Attrs : nounwind readonly
declare% dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32, % dx.types.Handle, i32) #2

; Function Attrs : nounwind readnone
declare i32 @dx.op.dispatchRaysDimensions.i32(i32, i8) #0

; Function Attrs : nounwind readnone
declare i32 @dx.op.dispatchRaysIndex.i32(i32, i8) #0

; Function Attrs : nounwind
declare void @dx.op.traceRay.struct.RayPayload(i32, % dx.types.Handle, i32, i32, i32, i32, i32, float, float, float, float, float, float, float, float, % struct.RayPayload*) #3

; Function Attrs : nounwind
declare void @dx.op.textureStore.f32(i32, % dx.types.Handle, i32, i32, i32, float, float, float, float, i8) #3

; Function Attrs : nounwind readonly
declare% dx.types.Handle @dx.op.createHandleForLib.g_rayGenCB(i32, % g_rayGenCB) #2

; Function Attrs : nounwind readonly
declare% dx.types.Handle @dx.op.createHandleForLib.struct.RaytracingAccelerationStructure(i32, % struct.RaytracingAccelerationStructure) #2

; Function Attrs : nounwind readonly
declare% dx.types.Handle @"dx.op.createHandleForLib.class.RWTexture2D<vector<float, 4> >"(i32, % "class.RWTexture2D<vector<float, 4> >") #2

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind "disable-tail-calls" = "false" "less-precise-fpmad" = "false" "no-frame-pointer-elim" = "false" "no-infs-fp-math" = "false" "no-nans-fp-math" = "false" "no-realign-stack" "stack-protector-buffer-size" = "0" "unsafe-fp-math" = "false" "use-soft-float" = "false" }
attributes #2 = { nounwind readonly }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!70, !71}
!llvm.ident = !{!72}
!dx.source.contents = !{!73, !74}
!dx.source.defines = !{!2}
!dx.source.mainFileName = !{!75}
!dx.source.args = !{!76}
!dx.version = !{!77}
!dx.valver = !{!78}
!dx.shaderModel = !{!79}
!dx.resources = !{!80}
!dx.typeAnnotations = !{!89, !113}
!dx.entryPoints = !{!120, !122, !125, !127}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file : !1, producer : "dxc 1.2", isOptimized : false, runtimeVersion : 0, emissionKind : 1, enums : !2, retainedTypes : !3, subprograms : !31, globals : !60)
!1 = !DIFile(filename: "Raytracing.hlsl", directory : "")
!2 = !{}
!3 = !{!4, !14, !22}
!4 = !DIDerivedType(tag: DW_TAG_typedef, name : "float2", file : !1, baseType : !5)
!5 = !DICompositeType(tag: DW_TAG_class_type, name : "vector<float, 2>", file : !1, size : 64, align : 32, elements : !6, templateParams : !10)
!6 = !{!7, !9}
!7 = !DIDerivedType(tag: DW_TAG_member, name : "x", scope : !5, file : !1, baseType : !8, size : 32, align : 32, flags : DIFlagPublic)
!8 = !DIBasicType(name: "float", size : 32, align : 32, encoding : DW_ATE_float)
!9 = !DIDerivedType(tag: DW_TAG_member, name : "y", scope : !5, file : !1, baseType : !8, size : 32, align : 32, offset : 32, flags : DIFlagPublic)
!10 = !{!11, !12}
!11 = !DITemplateTypeParameter(name: "element", type : !8)
!12 = !DITemplateValueParameter(name: "element_count", type : !13, value : i32 2)
!13 = !DIBasicType(name: "int", size : 32, align : 32, encoding : DW_ATE_signed)
!14 = !DIDerivedType(tag: DW_TAG_typedef, name : "float3", file : !1, line : 39, baseType : !15)
!15 = !DICompositeType(tag: DW_TAG_class_type, name : "vector<float, 3>", file : !1, line : 39, size : 96, align : 32, elements : !16, templateParams : !20)
!16 = !{!17, !18, !19}
!17 = !DIDerivedType(tag: DW_TAG_member, name : "x", scope : !15, file : !1, line : 39, baseType : !8, size : 32, align : 32, flags : DIFlagPublic)
!18 = !DIDerivedType(tag: DW_TAG_member, name : "y", scope : !15, file : !1, line : 39, baseType : !8, size : 32, align : 32, offset : 32, flags : DIFlagPublic)
!19 = !DIDerivedType(tag: DW_TAG_member, name : "z", scope : !15, file : !1, line : 39, baseType : !8, size : 32, align : 32, offset : 64, flags : DIFlagPublic)
!20 = !{!11, !21}
!21 = !DITemplateValueParameter(name: "element_count", type : !13, value : i32 3)
!22 = !DIDerivedType(tag: DW_TAG_typedef, name : "float4", file : !1, line : 56, baseType : !23)
!23 = !DICompositeType(tag: DW_TAG_class_type, name : "vector<float, 4>", file : !1, line : 56, size : 128, align : 32, elements : !24, templateParams : !29)
!24 = !{!25, !26, !27, !28}
!25 = !DIDerivedType(tag: DW_TAG_member, name : "x", scope : !23, file : !1, line : 56, baseType : !8, size : 32, align : 32, flags : DIFlagPublic)
!26 = !DIDerivedType(tag: DW_TAG_member, name : "y", scope : !23, file : !1, line : 56, baseType : !8, size : 32, align : 32, offset : 32, flags : DIFlagPublic)
!27 = !DIDerivedType(tag: DW_TAG_member, name : "z", scope : !23, file : !1, line : 56, baseType : !8, size : 32, align : 32, offset : 64, flags : DIFlagPublic)
!28 = !DIDerivedType(tag: DW_TAG_member, name : "w", scope : !23, file : !1, line : 56, baseType : !8, size : 32, align : 32, offset : 96, flags : DIFlagPublic)
!29 = !{!11, !30}
!30 = !DITemplateValueParameter(name: "element_count", type : !13, value : i32 4)
!31 = !{!32, !43, !46, !57}
!32 = !DISubprogram(name: "IsInsideViewport", linkageName : "\01?IsInsideViewport@@YA_NV?$vector@M$01@@UViewport@@@Z", scope : !1, file : !1, line : 27, type : !33, isLocal : false, isDefinition : true, scopeLine : 28, flags : DIFlagPrototyped, isOptimized : false)
!33 = !DISubroutineType(types: !34)
!34 = !{!35, !4, !36}
!35 = !DIBasicType(name: "bool", size : 32, align : 32, encoding : DW_ATE_boolean)
!36 = !DICompositeType(tag: DW_TAG_structure_type, name : "Viewport", file : !37, line : 15, size : 128, align : 32, elements : !38)
!37 = !DIFile(filename: "./RaytracingHlslCompat.h", directory : "")
!38 = !{!39, !40, !41, !42}
!39 = !DIDerivedType(tag: DW_TAG_member, name : "left", scope : !36, file : !37, line : 17, baseType : !8, size : 32, align : 32)
!40 = !DIDerivedType(tag: DW_TAG_member, name : "top", scope : !36, file : !37, line : 18, baseType : !8, size : 32, align : 32, offset : 32)
!41 = !DIDerivedType(tag: DW_TAG_member, name : "right", scope : !36, file : !37, line : 19, baseType : !8, size : 32, align : 32, offset : 64)
!42 = !DIDerivedType(tag: DW_TAG_member, name : "bottom", scope : !36, file : !37, line : 20, baseType : !8, size : 32, align : 32, offset : 96)
!43 = !DISubprogram(name: "MyRaygenShader", linkageName : "\01?MyRaygenShader@@YAXXZ", scope : !1, file : !1, line : 34, type : !44, isLocal : false, isDefinition : true, scopeLine : 35, flags : DIFlagPrototyped, isOptimized : false, function : void() * @"\01?MyRaygenShader@@YAXXZ")
!44 = !DISubroutineType(types: !45)
!45 = !{null}
!46 = !DISubprogram(name: "MyClosestHitShader", linkageName : "\01?MyClosestHitShader@@YAXURayPayload@@UBuiltInTriangleIntersectionAttributes@@@Z", scope : !1, file : !1, line : 70, type : !47, isLocal : false, isDefinition : true, scopeLine : 71, flags : DIFlagPrototyped, isOptimized : false, function : void(% struct.RayPayload*, % struct.BuiltInTriangleIntersectionAttributes*) * @"\01?MyClosestHitShader@@YAXURayPayload@@UBuiltInTriangleIntersectionAttributes@@@Z")
!47 = !DISubroutineType(types: !48)
!48 = !{null, !49, !53}
!49 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType : !50)
!50 = !DICompositeType(tag: DW_TAG_structure_type, name : "RayPayload", file : !1, line : 22, size : 128, align : 32, elements : !51)
!51 = !{!52}
!52 = !DIDerivedType(tag: DW_TAG_member, name : "color", scope : !50, file : !1, line : 24, baseType : !22, size : 128, align : 32)
!53 = !DIDerivedType(tag: DW_TAG_typedef, name : "MyAttributes", file : !1, line : 21, baseType : !54)
!54 = !DICompositeType(tag: DW_TAG_structure_type, name : "BuiltInTriangleIntersectionAttributes", file : !1, line : 67, size : 64, align : 32, elements : !55)
!55 = !{!56}
!56 = !DIDerivedType(tag: DW_TAG_member, name : "barycentrics", scope : !54, file : !1, line : 67, baseType : !5, size : 64, align : 32)
!57 = !DISubprogram(name: "MyMissShader", linkageName : "\01?MyMissShader@@YAXURayPayload@@@Z", scope : !1, file : !1, line : 77, type : !58, isLocal : false, isDefinition : true, scopeLine : 78, flags : DIFlagPrototyped, isOptimized : false, function : void(% struct.RayPayload*) * @"\01?MyMissShader@@YAXURayPayload@@@Z")
!58 = !DISubroutineType(types: !59)
!59 = !{null, !49}
!60 = !{!61, !64, !66}
!61 = !DIGlobalVariable(name: "RAY_FLAG_CULL_BACK_FACING_TRIANGLES", scope : !0, file : !1, line : 57, type : !62, isLocal : true, isDefinition : true, variable : i32 16)
!62 = !DIDerivedType(tag: DW_TAG_const_type, baseType : !63)
!63 = !DIBasicType(name: "unsigned int", size : 32, align : 32, encoding : DW_ATE_unsigned)
!64 = !DIGlobalVariable(name: "Scene", linkageName : "\01?Scene@@3URaytracingAccelerationStructure@@A", scope : !0, file : !1, line : 17, type : !65, isLocal : false, isDefinition : true, variable : % struct.RaytracingAccelerationStructure * @"\01?Scene@@3URaytracingAccelerationStructure@@A")
!65 = !DICompositeType(tag: DW_TAG_structure_type, name : "RaytracingAccelerationStructure", file : !1, line : 17, size : 32, align : 32, elements : !2)
!66 = !DIGlobalVariable(name: "RenderTarget", linkageName : "\01?RenderTarget@@3V?$RWTexture2D@V?$vector@M$03@@@@A", scope : !0, file : !1, line : 18, type : !67, isLocal : false, isDefinition : true, variable : % "class.RWTexture2D<vector<float, 4> >" * @"\01?RenderTarget@@3V?$RWTexture2D@V?$vector@M$03@@@@A")
!67 = !DICompositeType(tag: DW_TAG_class_type, name : "RWTexture2D<vector<float, 4> >", file : !1, line : 18, size : 128, align : 32, elements : !2, templateParams : !68)
!68 = !{!69}
!69 = !DITemplateTypeParameter(name: "element", type : !23)
!70 = !{i32 2, !"Dwarf Version", i32 4}
!71 = !{i32 2, !"Debug Info Version", i32 3}
!72 = !{!"dxc 1.2"}
!73 = !{!"Raytracing.hlsl", !"//*********************************************************\0D\0A//\0D\0A// Copyright (c) Microsoft. All rights reserved.\0D\0A// This code is licensed under the MIT License (MIT).\0D\0A// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF\0D\0A// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY\0D\0A// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR\0D\0A// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.\0D\0A//\0D\0A//*********************************************************\0D\0A\0D\0A#ifndef RAYTRACING_HLSL\0D\0A#define RAYTRACING_HLSL\0D\0A\0D\0A#include \22RaytracingHlslCompat.h\22\0D\0A\0D\0ARaytracingAccelerationStructure Scene : register(t0, space0);\0D\0ARWTexture2D<float4> RenderTarget : register(u0);\0D\0AConstantBuffer<RayGenConstantBuffer> g_rayGenCB : register(b0);\0D\0A\0D\0Atypedef BuiltInTriangleIntersectionAttributes MyAttributes;\0D\0Astruct RayPayload\0D\0A{\0D\0A    float4 color;\0D\0A};\0D\0A\0D\0Abool IsInsideViewport(float2 p, Viewport viewport)\0D\0A{\0D\0A    return (p.x >= viewport.left && p.x <= viewport.right)\0D\0A        && (p.y >= viewport.top && p.y <= viewport.bottom);\0D\0A}\0D\0A\0D\0A[shader(\22raygeneration\22)]\0D\0Avoid MyRaygenShader()\0D\0A{\0D\0A    float2 lerpValues = (float2)DispatchRaysIndex() / (float2)DispatchRaysDimensions();\0D\0A\0D\0A    // Orthographic projection since we're raytracing in screen space.\0D\0A    float3 rayDir = float3(0, 0, 1);\0D\0A    float3 origin = float3(\0D\0A        lerp(g_rayGenCB.viewport.left, g_rayGenCB.viewport.right, lerpValues.x),\0D\0A        lerp(g_rayGenCB.viewport.top, g_rayGenCB.viewport.bottom, lerpValues.y),\0D\0A        0.0f);\0D\0A\0D\0A    if (IsInsideViewport(origin.xy, g_rayGenCB.stencil))\0D\0A    {\0D\0A        // Trace the ray.\0D\0A        // Set the ray's extents.\0D\0A        RayDesc ray;\0D\0A        ray.Origin = origin;\0D\0A        ray.Direction = rayDir;\0D\0A        // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.\0D\0A        // TMin should be kept small to prevent missing geometry at close contact areas.\0D\0A        ray.TMin = 0.001;\0D\0A        ray.TMax = 10000.0;\0D\0A        RayPayload payload = { float4(0, 0, 0, 0) };\0D\0A        TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);\0D\0A\0D\0A        // Write the raytraced color to the output texture.\0D\0A        RenderTarget[DispatchRaysIndex().xy] = payload.color;\0D\0A    }\0D\0A    else\0D\0A    {\0D\0A        // Render interpolated DispatchRaysIndex outside the stencil window\0D\0A        RenderTarget[DispatchRaysIndex().xy] = float4(lerpValues, 0, 1);\0D\0A    }\0D\0A}\0D\0A\0D\0A[shader(\22closesthit\22)]\0D\0Avoid MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)\0D\0A{\0D\0A    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);\0D\0A    payload.color = float4(barycentrics, 1);\0D\0A}\0D\0A\0D\0A[shader(\22miss\22)]\0D\0Avoid MyMissShader(inout RayPayload payload)\0D\0A{\0D\0A    payload.color = float4(0, 0, 0, 1);\0D\0A}\0D\0A\0D\0A#endif // RAYTRACING_HLSL"}
!74 = !{!"./RaytracingHlslCompat.h", !"//*********************************************************\0D\0A//\0D\0A// Copyright (c) Microsoft. All rights reserved.\0D\0A// This code is licensed under the MIT License (MIT).\0D\0A// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF\0D\0A// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY\0D\0A// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR\0D\0A// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.\0D\0A//\0D\0A//*********************************************************\0D\0A\0D\0A#ifndef RAYTRACINGHLSLCOMPAT_H\0D\0A#define RAYTRACINGHLSLCOMPAT_H\0D\0A\0D\0Astruct Viewport\0D\0A{\0D\0A    float left;\0D\0A    float top;\0D\0A    float right;\0D\0A    float bottom;\0D\0A};\0D\0A\0D\0Astruct RayGenConstantBuffer\0D\0A{\0D\0A    Viewport viewport;\0D\0A    Viewport stencil;\0D\0A};\0D\0A\0D\0A#endif // RAYTRACINGHLSLCOMPAT_H"}
!75 = !{!"Raytracing.hlsl"}
!76 = !{!"/T", !"lib_6_3", !"/Fo", !"bin\5Cx64\5CDebug\5CRaytracing.cso", !"/Od", !"/Zi"}
!77 = !{i32 1, i32 3}
!78 = !{i32 1, i32 5}
!79 = !{!"lib", i32 6, i32 3}
!80 = !{!81, !84, !87, null}
!81 = !{!82}
!82 = !{i32 0, % struct.RaytracingAccelerationStructure* @"\01?Scene@@3URaytracingAccelerationStructure@@A", !"Scene", i32 0, i32 0, i32 1, i32 16, i32 0, !83}
!83 = !{i32 0, i32 4}
!84 = !{!85}
!85 = !{i32 0, % "class.RWTexture2D<vector<float, 4> >" * @"\01?RenderTarget@@3V?$RWTexture2D@V?$vector@M$03@@@@A", !"RenderTarget", i32 0, i32 0, i32 1, i32 2, i1 false, i1 false, i1 false, !86}
!86 = !{i32 0, i32 9}
!87 = !{!88}
!88 = !{i32 0, % g_rayGenCB* @g_rayGenCB, !"g_rayGenCB", i32 0, i32 0, i32 1, i32 32, null}
!89 = !{i32 0, % struct.RaytracingAccelerationStructure undef, !90, % "class.RWTexture2D<vector<float, 4> >" undef, !92, % struct.RayGenConstantBuffer undef, !94, % struct.Viewport undef, !97, % struct.RayDesc undef, !102, % struct.RayPayload undef, !107, % struct.BuiltInTriangleIntersectionAttributes undef, !109, % g_rayGenCB undef, !111}
!90 = !{i32 4, !91}
!91 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 4}
!92 = !{i32 16, !93}
!93 = !{i32 6, !"h", i32 3, i32 0, i32 7, i32 9}
!94 = !{i32 32, !95, !96}
!95 = !{i32 6, !"viewport", i32 3, i32 0}
!96 = !{i32 6, !"stencil", i32 3, i32 16}
!97 = !{i32 16, !98, !99, !100, !101}
!98 = !{i32 6, !"left", i32 3, i32 0, i32 7, i32 9}
!99 = !{i32 6, !"top", i32 3, i32 4, i32 7, i32 9}
!100 = !{i32 6, !"right", i32 3, i32 8, i32 7, i32 9}
!101 = !{i32 6, !"bottom", i32 3, i32 12, i32 7, i32 9}
!102 = !{i32 32, !103, !104, !105, !106}
!103 = !{i32 6, !"Origin", i32 3, i32 0, i32 7, i32 9}
!104 = !{i32 6, !"TMin", i32 3, i32 12, i32 7, i32 9}
!105 = !{i32 6, !"Direction", i32 3, i32 16, i32 7, i32 9}
!106 = !{i32 6, !"TMax", i32 3, i32 28, i32 7, i32 9}
!107 = !{i32 16, !108}
!108 = !{i32 6, !"color", i32 3, i32 0, i32 7, i32 9}
!109 = !{i32 8, !110}
!110 = !{i32 6, !"barycentrics", i32 3, i32 0, i32 7, i32 9}
!111 = !{i32 32, !112}
!112 = !{i32 6, !"g_rayGenCB", i32 3, i32 0}
!113 = !{i32 1, void()* @"\01?MyRaygenShader@@YAXXZ", !114, void(% struct.RayPayload*, % struct.BuiltInTriangleIntersectionAttributes*)* @"\01?MyClosestHitShader@@YAXURayPayload@@UBuiltInTriangleIntersectionAttributes@@@Z", !116, void(% struct.RayPayload*)* @"\01?MyMissShader@@YAXURayPayload@@@Z", !119}
!114 = !{!115}
!115 = !{i32 1, !2, !2}
!116 = !{!115, !117, !118}
!117 = !{i32 2, !2, !2}
!118 = !{i32 0, !2, !2}
!119 = !{!115, !117}
!120 = !{null, !"", null, !80, !121}
!121 = !{i32 0, i64 65537}
!122 = !{void(% struct.RayPayload*, % struct.BuiltInTriangleIntersectionAttributes*)* @"\01?MyClosestHitShader@@YAXURayPayload@@UBuiltInTriangleIntersectionAttributes@@@Z", !"\01?MyClosestHitShader@@YAXURayPayload@@UBuiltInTriangleIntersectionAttributes@@@Z", null, null, !123}
!123 = !{i32 8, i32 10, i32 6, i32 16, i32 7, i32 8, i32 5, !124}
!124 = !{i32 0}
!125 = !{void(% struct.RayPayload*)* @"\01?MyMissShader@@YAXURayPayload@@@Z", !"\01?MyMissShader@@YAXURayPayload@@@Z", null, null, !126}
!126 = !{i32 8, i32 11, i32 6, i32 16, i32 5, !124}
!127 = !{void()* @"\01?MyRaygenShader@@YAXXZ", !"\01?MyRaygenShader@@YAXXZ", null, null, !128}
!128 = !{i32 8, i32 7, i32 5, !124}
!129 = !DILocalVariable(tag: DW_TAG_auto_variable, name : "lerpValues", scope : !43, file : !1, line : 36, type : !4)
!130 = !DIExpression()
!131 = !DILocation(line: 36, column : 12, scope : !43)
!132 = !DILocation(line: 36, column : 33, scope : !43)
!133 = !DILocation(line: 36, column : 63, scope : !43)
!134 = !DILocation(line: 36, column : 53, scope : !43)
!135 = !DILocalVariable(tag: DW_TAG_auto_variable, name : "rayDir", scope : !43, file : !1, line : 39, type : !14)
!136 = !DILocation(line: 39, column : 12, scope : !43)
!137 = !DILocalVariable(tag: DW_TAG_auto_variable, name : "origin", scope : !43, file : !1, line : 40, type : !14)
!138 = !DILocation(line: 40, column : 12, scope : !43)
!139 = !DILocation(line: 41, column : 67, scope : !43)
!140 = !DILocation(line: 41, column : 60, scope : !43)
!141 = !DILocation(line: 41, column : 34, scope : !43)
!142 = !DILocation(line: 41, column : 9, scope : !43)
!143 = !DILocation(line: 42, column : 67, scope : !43)
!144 = !DILocation(line: 42, column : 59, scope : !43)
!145 = !DILocation(line: 42, column : 34, scope : !43)
!146 = !DILocation(line: 42, column : 9, scope : !43)
!147 = !DILocation(line: 45, column : 26, scope : !148)
!148 = distinct !DILexicalBlock(scope: !43, file : !1, line : 45, column : 9)
!149 = !DILocation(line: 45, column : 9, scope : !148)
!150 = !DILocation(line: 29, column : 13, scope : !32, inlinedAt : !151)
!151 = distinct !DILocation(line: 45, column : 9, scope : !148)
!152 = !DILocation(line: 29, column : 29, scope : !32, inlinedAt : !151)
!153 = !DILocation(line: 29, column : 17, scope : !32, inlinedAt : !151)
!154 = !DILocation(line: 29, column : 37, scope : !32, inlinedAt : !151)
!155 = !DILocation(line: 29, column : 53, scope : !32, inlinedAt : !151)
!156 = !DILocation(line: 29, column : 41, scope : !32, inlinedAt : !151)
!157 = !DILocation(line: 29, column : 34, scope : !32, inlinedAt : !151)
!158 = !DILocation(line: 30, column : 13, scope : !32, inlinedAt : !151)
!159 = !DILocation(line: 30, column : 29, scope : !32, inlinedAt : !151)
!160 = !DILocation(line: 30, column : 17, scope : !32, inlinedAt : !151)
!161 = !DILocation(line: 30, column : 36, scope : !32, inlinedAt : !151)
!162 = !DILocation(line: 30, column : 52, scope : !32, inlinedAt : !151)
!163 = !DILocation(line: 30, column : 40, scope : !32, inlinedAt : !151)
!164 = !DILocation(line: 30, column : 33, scope : !32, inlinedAt : !151)
!165 = !DILocation(line: 30, column : 9, scope : !32, inlinedAt : !151)
!166 = !DILocation(line: 29, column : 5, scope : !32, inlinedAt : !151)
!167 = !DILocalVariable(tag: DW_TAG_arg_variable, name : "p", arg : 1, scope : !32, file : !1, line : 27, type : !4)
!168 = !DILocation(line: 27, column : 30, scope : !32, inlinedAt : !151)
!169 = !DILocation(line: 45, column : 9, scope : !43)
!170 = !DILocalVariable(tag: DW_TAG_auto_variable, name : "ray", scope : !171, file : !1, line : 49, type : !172)
!171 = distinct !DILexicalBlock(scope: !148, file : !1, line : 46, column : 5)
!172 = !DICompositeType(tag: DW_TAG_structure_type, name : "RayDesc", file : !1, line : 49, size : 256, align : 32, elements : !173)
!173 = !{!174, !175, !176, !177}
!174 = !DIDerivedType(tag: DW_TAG_member, name : "Origin", scope : !172, file : !1, line : 49, baseType : !15, size : 96, align : 32)
!175 = !DIDerivedType(tag: DW_TAG_member, name : "TMin", scope : !172, file : !1, line : 49, baseType : !8, size : 32, align : 32, offset : 96)
!176 = !DIDerivedType(tag: DW_TAG_member, name : "Direction", scope : !172, file : !1, line : 49, baseType : !15, size : 96, align : 32, offset : 128)
!177 = !DIDerivedType(tag: DW_TAG_member, name : "TMax", scope : !172, file : !1, line : 49, baseType : !8, size : 32, align : 32, offset : 224)
!178 = !DILocation(line: 49, column : 17, scope : !171)
!179 = !DILocation(line: 50, column : 22, scope : !171)
!180 = !DILocation(line: 50, column : 13, scope : !171)
!181 = !DILocation(line: 50, column : 20, scope : !171)
!182 = !DILocation(line: 51, column : 25, scope : !171)
!183 = !DILocation(line: 51, column : 13, scope : !171)
!184 = !DILocation(line: 51, column : 23, scope : !171)
!185 = !DILocation(line: 54, column : 13, scope : !171)
!186 = !DILocation(line: 54, column : 18, scope : !171)
!187 = !DILocation(line: 55, column : 13, scope : !171)
!188 = !DILocation(line: 55, column : 18, scope : !171)
!189 = !DILocalVariable(tag: DW_TAG_auto_variable, name : "payload", scope : !171, file : !1, line : 56, type : !50)
!190 = !DILocation(line: 56, column : 20, scope : !171)
!191 = !DILocation(line: 56, column : 30, scope : !171)
!192 = !DILocation(line: 57, column : 9, scope : !171)
!193 = !DILocation(line: 60, column : 56, scope : !171)
!194 = !DILocation(line: 60, column : 22, scope : !171)
!195 = !DILocation(line: 60, column : 9, scope : !171)
!196 = !DILocation(line: 60, column : 46, scope : !171)
!197 = !DILocation(line: 61, column : 5, scope : !171)
!198 = !DILocation(line: 65, column : 55, scope : !199)
!199 = distinct !DILexicalBlock(scope: !148, file : !1, line : 63, column : 5)
!200 = !DILocation(line: 65, column : 22, scope : !199)
!201 = !DILocation(line: 65, column : 9, scope : !199)
!202 = !DILocation(line: 65, column : 46, scope : !199)
!203 = !DILocation(line: 67, column : 1, scope : !43)
!204 = !DILocalVariable(tag: DW_TAG_arg_variable, name : "attr", arg : 2, scope : !46, file : !1, line : 70, type : !53)
!205 = !DIExpression(DW_OP_deref)
!206 = !DILocation(line: 70, column : 67, scope : !46)
!207 = !DILocalVariable(tag: DW_TAG_arg_variable, name : "payload", arg : 1, scope : !46, file : !1, line : 70, type : !49)
!208 = !DILocation(line: 70, column : 42, scope : !46)
!209 = !DILocalVariable(tag: DW_TAG_auto_variable, name : "barycentrics", scope : !46, file : !1, line : 72, type : !14)
!210 = !DILocation(line: 72, column : 12, scope : !46)
!211 = !DILocation(line: 72, column : 43, scope : !46)
!212 = !DILocation(line: 72, column : 38, scope : !46)
!213 = !DILocation(line: 72, column : 36, scope : !46)
!214 = !DILocation(line: 72, column : 65, scope : !46)
!215 = !DILocation(line: 72, column : 60, scope : !46)
!216 = !DILocation(line: 72, column : 58, scope : !46)
!217 = !DILocation(line: 72, column : 86, scope : !46)
!218 = !DILocation(line: 72, column : 81, scope : !46)
!219 = !DILocation(line: 72, column : 107, scope : !46)
!220 = !DILocation(line: 72, column : 102, scope : !46)
!221 = !DILocation(line: 73, column : 28, scope : !46)
!222 = !DILocation(line: 73, column : 19, scope : !46)
!223 = !DILocation(line: 74, column : 1, scope : !46)
!224 = !DILocation(line: 77, column : 36, scope : !57)
!225 = !DILocalVariable(tag: DW_TAG_arg_variable, name : "payload", arg : 1, scope : !57, file : !1, line : 77, type : !49)
!226 = !DILocation(line: 79, column : 13, scope : !57)
!227 = !DILocation(line: 79, column : 19, scope : !57)
!228 = !DILocation(line: 80, column : 1, scope : !57)

#endif
